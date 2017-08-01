#include <ModelViewer.h>
#include <FeatureUtils.h>                       // RFeatures
#include <VtkActorCreator.h>                    // RVTK
#include <ImageGrabber.h>                       // RVTK
#include <vtkInteractorStyleTrackballCamera.h>  // VTK
#include <vtkMapper.h>                          // VTK
#include <vtkProperty.h>                        // VTK
#include <boost/foreach.hpp>
using FaceTools::ModelViewer;
using RFeatures::ObjModel;
using RFeatures::CameraParams;


// public
void ModelViewer::updateRender()
{
    _viewer->updateRender();
}   // end updateRender


// public
void ModelViewer::enableFloodLights( bool enable)
{
    if ( !enable)
        _viewer->setHeadlight();
    else
    {
        static const float slr = 600;
        std::vector<cv::Vec3f> lpos(6);
        std::vector<cv::Vec3f> lfoc( 6, cv::Vec3f(0,0,0));
        lpos[0] = cv::Vec3f(    0,    0,  slr); // point at face from front
        lpos[1] = cv::Vec3f( -slr,    0,    0); // 90 degrees to left of face
        lpos[2] = cv::Vec3f(  slr,    0,    0); // 90 degrees to right of face
        lpos[3] = cv::Vec3f(    0, -slr,    0); // 90 degrees below face
        lpos[4] = cv::Vec3f(    0,  slr,    0); // 90 degrees above face
        lpos[5] = cv::Vec3f(    0,    0, -slr); // point at face from behind
        _viewer->setSceneLights( lpos, lfoc);
    }   // end if
}   // end enableFloodLights


// public
void ModelViewer::enableAxes( bool enable)
{
    if ( enable)
        _axes->show();
    else
        _axes->hide();
}   // end enableAxes


// private
void ModelViewer::init()
{
    _scalarLegend = new RVTK::ScalarLegend( _viewer->getRenderer());
    _axes = new RVTK::Axes( _viewer->getRenderer());
    _axes->setInteractor( _viewer->GetInteractor());
    _axes->hide();
}   // end init


// public
ModelViewer::ModelViewer( const cv::Size& vsz, bool floodFill, bool offscreen)
    : _viewer( new QTools::VtkActorViewer( NULL, offscreen)), _scalarLegend(NULL), _axes(NULL), _dodel(true), _addedModelID(0)
{
    vtkObject::GlobalWarningDisplayOff();   // Prevent GUI error warning pop-ups
    vtkSmartPointer<vtkRenderer> renderer = _viewer->getRenderer();
    renderer->SetGradientBackground(false);
    renderer->SetBackground(0,0,0);
    _viewer->setInteractor( vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
    _viewer->setSize( vsz.width, vsz.height);
    enableFloodLights( floodFill);
    init();
}   // end ctor


// public
ModelViewer::ModelViewer( QTools::VtkActorViewer* viewer)
    : _viewer( viewer), _scalarLegend(NULL), _axes(NULL), _dodel(false), _addedModelID(0)
{
    init();
}   // end ctor


// public
ModelViewer::~ModelViewer()
{
    delete _axes;
    delete _scalarLegend;
    if ( _dodel)
        delete _viewer;
}   // end dtor


// public
void ModelViewer::show()
{
    _viewer->show();
}   // end show


// public
void ModelViewer::hide()
{
    _viewer->hide();
}   // end hide


cv::Point2f ModelViewer::projectProp( const cv::Vec3f& v) const
{
    return _viewer->projectToDisplayProportion( v);
}   // end projectProp


cv::Point ModelViewer::project( const cv::Vec3f& v) const
{
    return _viewer->projectToDisplay( v);
}   // end project


cv::Vec3f ModelViewer::project( const cv::Point2f& p) const
{
    return _viewer->pickWorldPosition( p);
}   // end project


cv::Vec3f ModelViewer::project( const cv::Point& p) const
{
    return _viewer->pickWorldPosition( p);
}   // end project


int ModelViewer::add( const ObjModel::Ptr model, const ModelViewer::VisOptions& vo)
{
    // If a textured model was selected but the model has no texture, set the wireframe visualisation instead.
    ModelViewer::Visualisation mvis = vo.vis;
    if ( mvis == VisTexture && model->getNumMaterials() == 0)
    {
        std::cerr << "ModelViewer: Texture visualisation requested but not texture present!" << std::endl;
        mvis = VisWireframe;
    }   // end if

    RVTK::VtkActorCreator actorCreator;
    std::vector<vtkSmartPointer<vtkActor> > actors;
    if ( mvis == VisTexture)
        actorCreator.generateTexturedActors( model, actors);
    else
    {
        vtkSmartPointer<vtkActor> actor = actorCreator.generateSurfaceActor( model);
        switch ( mvis)
        {
            case VisPoints:
                actor->GetProperty()->SetRepresentationToPoints();
                actor->GetProperty()->SetPointSize(vo.pointSize);
                break;
            case VisWireframe:
                actor->GetProperty()->SetRepresentationToWireframe();
                break;
            case VisSurface:
                actor->GetProperty()->SetRepresentationToSurface();
                break;
        }   // end switch

        actor->GetProperty()->SetOpacity( vo.a);
        actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);
        actor->GetMapper()->SetScalarVisibility(true);
        actors.push_back( actor);
    }   // end if

    const int modelID = _addedModelID++;
    BOOST_FOREACH ( const vtkSmartPointer<vtkActor> actor, actors)
    {
        actor->GetProperty()->SetBackfaceCulling( vo.backfaceCulling);
        _actors[modelID].push_back(actor);
        _viewer->addActor( actor);
    }   // end foreach

    _scalarLegend->hide();
    return modelID;
}   // end add


// public
int ModelViewer::add( RVTK::MetricInterface* mint, float minv, const cv::Vec3b& col0, float maxv, const cv::Vec3b& col1, int ncols, bool showLegend)
{
    vtkSmartPointer<vtkActor> actor = mint->getMapper()->createSurfaceActor( mint);

    const vtkColor3ub minCol( col0[0], col0[1], col0[2]);
    const vtkColor3ub maxCol( col1[0], col1[1], col1[2]);
    _scalarLegend->setTitle( mint->getMetricName());
    _scalarLegend->setColours( ncols, minCol, maxCol);
    _scalarLegend->setLookupTable( actor->GetMapper(), minv, maxv);

    const int modelID = _addedModelID++;
    _actors[modelID].push_back(actor);
    _viewer->addActor(actor);

    if ( showLegend)
        _scalarLegend->show();
    return modelID;
}   // end add


// private
int ModelViewer::addPointsActor( vtkSmartPointer<vtkActor> actor, const ModelViewer::VisOptions& vo)
{
    actor->GetProperty()->SetRepresentationToPoints();
    actor->GetProperty()->SetRenderPointsAsSpheres(true);
    actor->GetProperty()->SetPointSize( vo.pointSize);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);

    const int modelID = _addedModelID++;
    _actors[modelID].push_back(actor);
    _viewer->addActor(actor);
    return modelID;
}   // end addPointsActor


int ModelViewer::addPoints( const std::vector<cv::Vec3f>& vpts, const ModelViewer::VisOptions& vo)
{
    return addPointsActor( RVTK::VtkActorCreator::generatePointsActor( vpts), vo);
}   // end addPoints


int ModelViewer::addPoints( const ObjModel::Ptr model, const ModelViewer::VisOptions& vo)
{
    return addPointsActor( RVTK::VtkActorCreator::generatePointsActor( model), vo);
}   // end addPoints


int ModelViewer::addPoints( const ObjModel::Ptr model, const IntSet& vidxs, const ModelViewer::VisOptions& vo)
{
    std::vector<cv::Vec3f> vpts(vidxs.size());
    int i = 0;
    BOOST_FOREACH ( int vidx, vidxs)
        vpts[i++] = model->vtx(vidx);
    return addPoints( vpts, vo);
}   // end addPoints


int ModelViewer::addLine( const std::vector<cv::Vec3f>& vpts, bool joinEnds, const ModelViewer::VisOptions& vo)
{
    vtkSmartPointer<vtkActor> actor = RVTK::VtkActorCreator::generateLineActor( vpts, joinEnds);
    actor->GetProperty()->SetRepresentationToWireframe();
    actor->GetProperty()->SetRenderPointsAsSpheres(true);
    actor->GetProperty()->SetPointSize(vo.pointSize);
    actor->GetProperty()->SetLineWidth(vo.lineWidth);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);

    const int modelID = _addedModelID++;
    _actors[modelID].push_back(actor);
    _viewer->addActor(actor);
    return modelID;
}   // end addLine


int ModelViewer::addLinePairs( const std::vector<cv::Vec3f>& lps, const ModelViewer::VisOptions& vo)
{
    vtkSmartPointer<vtkActor> actor = RVTK::VtkActorCreator::generateLinePairsActor( lps);
    actor->GetProperty()->SetRepresentationToWireframe();
    actor->GetProperty()->SetRenderPointsAsSpheres(true);
    actor->GetProperty()->SetPointSize(vo.pointSize);
    actor->GetProperty()->SetLineWidth(vo.lineWidth);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);

    const int modelID = _addedModelID++;
    _actors[modelID].push_back(actor);
    _viewer->addActor(actor);
    return modelID;
}   // end addLinePairs


bool ModelViewer::remove( int modelID)
{
    if ( !_actors.count(modelID))
        return false;

    const std::vector<vtkSmartPointer<vtkActor> >& actors = _actors.at(modelID);
    BOOST_FOREACH ( vtkSmartPointer<vtkActor> actor, actors)
        _viewer->removeActor( actor);
    _actors.erase(modelID);
    return true;
}   // end remove


void ModelViewer::removeAll()
{
    _viewer->clear();
    _actors.clear();
}   // end removeAll


CameraParams ModelViewer::getCamera() const
{
    CameraParams cp;
    _viewer->getCamera( cp);
    return cp;
}   // end getCamera


void ModelViewer::setCamera( const CameraParams& cp)
{
    _viewer->setCamera( cp);
}   // end setCamera


cv::Size_<int> ModelViewer::getViewportSize() const
{
    return _viewer->getSize();
}   // end getViewportSize


void ModelViewer::resetDefaultCamera( float camRng)
{
    // Set initial camera params
    CameraParams cp = getCamera();
    cp.fov = 30;
    cp.focus = cv::Vec3f(0,0,0);
    cp.pos = cp.focus;
    cp.pos[2] += camRng; // Set the camera position to be directly infront of the focus
    setCamera(cp);
}   // end resetDefaultCamera


void ModelViewer::setCamera( const cv::Vec3f& focus, const cv::Vec3f& nvec, const cv::Vec3f& uvec, float camRng)
{
    const cv::Vec3f pos = focus + camRng*nvec;
    const CameraParams cp( pos, focus, uvec);
    setCamera( cp);
}   // end setCamera


void ModelViewer::fitCamera( double radius)
{
    // Set new field of view based on params.faceCropRadius unless custom field of view set
    CameraParams cp = getCamera();
    const double crng = cv::norm(cp.focus - cp.pos);
    cp.fov = atan2( radius, crng) * 360.0/CV_PI;
    setCamera(cp);
}   // end fitCamera


void ModelViewer::showSnapshot( bool waitForInput)
{
    updateRender();
    RVTK::ImageGrabber imgGrabber( _viewer->GetRenderWindow());
    imgGrabber.update();
    RFeatures::showImage( imgGrabber.getColourMap(), "SNAPSHOT", waitForInput);
}   // end showSnapshot
