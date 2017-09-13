/************************************************************************
 * Copyright (C) 2017 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <ModelViewer.h>
#include <MiscFunctions.h>
#include <FeatureUtils.h>                       // RFeatures
#include <VtkTools.h>                           // RVTK
#include <VtkActorCreator.h>                    // RVTK
#include <ImageGrabber.h>                       // RVTK
#include <PointPlacer.h>                        // RVTK
#include <QImageTools.h>                        // QTools
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
    _qviewer->updateRender();
}   // end updateRender


// public
void ModelViewer::enableFloodLights( bool enable)
{
    std::vector<RVTK::Light> lights;
    if ( enable)
        RVTK::createBoxLights( 600, lights);
    else
        lights.push_back( RVTK::Light()); // Default RVTK::Light is a bright white headlight
    _qviewer->setLights( lights);
}   // end enableFloodLights


// public
void ModelViewer::showAxes( bool enable)
{
    if ( enable)
        _axes->show();
    else
        _axes->hide();
}   // end showAxes


// public
void ModelViewer::showLegend( bool enable)
{
    if ( enable)
        _scalarLegend->show();
    else
        _scalarLegend->hide();
}   // end showLegend


// private
void ModelViewer::init()
{
    _scalarLegend = new RVTK::ScalarLegend( _qviewer->getRenderer());
    _axes = new RVTK::Axes( _qviewer->getRenderer());
    _axes->setInteractor( _qviewer->GetInteractor());
    _axes->hide();
}   // end init


// public
ModelViewer::ModelViewer( bool floodFill)
    : _qviewer( new QTools::VtkActorViewer( NULL)), _scalarLegend(NULL), _axes(NULL), _dodel(true), _addedModelID(0)
{
    vtkObject::GlobalWarningDisplayOff();   // Prevent GUI error warning pop-ups
    vtkSmartPointer<vtkRenderer> renderer = _qviewer->getRenderer();
    renderer->SetGradientBackground(false);
    renderer->SetBackground(0,0,0);
    _qviewer->setInteractor( vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
    enableFloodLights( floodFill);
    init();
}   // end ctor


// public
ModelViewer::ModelViewer( QTools::VtkActorViewer* viewer)
    : _qviewer( viewer), _scalarLegend(NULL), _axes(NULL), _dodel(false), _addedModelID(0)
{
    init();
}   // end ctor


// public
ModelViewer::~ModelViewer()
{
    delete _axes;
    delete _scalarLegend;
    if ( _dodel)
        delete _qviewer;
}   // end dtor


// public
void ModelViewer::setSize( const cv::Size& sz)
{
    _qviewer->setSize( sz.width, sz.height);
}   // end setSize


// public
void ModelViewer::show()
{
    _qviewer->show();
}   // end show


// public
void ModelViewer::hide()
{
    _qviewer->hide();
}   // end hide


// public
cv::Point2f ModelViewer::projectProp( const cv::Vec3f& v) const
{
    return _qviewer->projectToDisplayProportion( v);
}   // end projectProp


// public
cv::Point ModelViewer::project( const cv::Vec3f& v) const
{
    return _qviewer->projectToDisplay( v);
}   // end project


// public
cv::Vec3f ModelViewer::project( const cv::Point2f& p) const
{
    return _qviewer->pickWorldPosition( p);
}   // end project


// public
cv::Vec3f ModelViewer::project( const cv::Point& p) const
{
    return _qviewer->pickWorldPosition( p);
}   // end project


// public
const vtkProp* ModelViewer::getPointedAt( const cv::Point2f& p) const
{
    const cv::Point preal = FaceTools::fromProportion( p, cv::Size2i( getWidth(), getHeight()));
    return getPointedAt(preal);
}   // end getPointedAt


// public
const vtkProp* ModelViewer::getPointedAt( const cv::Point& p) const
{
    return _qviewer->pickActor( p);
}   // end getPointedAt


// public
const vtkProp* ModelViewer::getPointedAt( const QPoint& q) const
{
    const cv::Point p(q.x(), q.y());
    return _qviewer->pickActor( p);
}   // end getPointedAt


// public
bool ModelViewer::calcSurfacePosition( const vtkProp *prop, const QPoint& q, cv::Vec3f& worldPos) const
{
    const cv::Point p(q.x(), q.y());
    return calcSurfacePosition( prop, p, worldPos);
}   // end calcSurfacePosition


// public
bool ModelViewer::calcSurfacePosition( const vtkProp *prop, const cv::Point& p, cv::Vec3f& worldPos) const
{
    if ( !prop)
        return false;
    RVTK::PointPlacer::Ptr pplacer = RVTK::PointPlacer::create( _qviewer->getRenderer());
    pplacer->set(prop);
    return pplacer->calcSurfacePosition( p.x, p.y, &worldPos[0], RVTK::TOP_LEFT_DISPLAY_ORIGIN);
}   // end calcSurfacePosition


// public
bool ModelViewer::calcSurfacePosition( const vtkProp *prop, const cv::Point2f& pf, cv::Vec3f& worldPos) const
{
    const cv::Point p = FaceTools::fromProportion( pf, cv::Size2i( getWidth(), getHeight()));
    return calcSurfacePosition( prop, p, worldPos);
}   // end calcSurfacePosition


// public
void ModelViewer::setCursor( QCursor cursor)
{
    _qviewer->setCursor(cursor);
}   // end setCursor


// public
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
        _props[modelID].push_back(actor);
        add( actor);
    }   // end foreach

    showLegend(false);
    return modelID;
}   // end add


// public
int ModelViewer::add( RVTK::SurfaceMapper* smapper, float minv, float maxv)
{
    vtkSmartPointer<vtkActor> actor = smapper->makeActor();
    return add( actor, smapper->getMetricName(), minv, maxv);
}   // end add


// public
int ModelViewer::add( vtkSmartPointer<vtkActor> actor, const std::string& ltitle, float minv, float maxv)
{
    _scalarLegend->setTitle( ltitle);
    _scalarLegend->setLookupTable( actor->GetMapper(), minv, maxv);

    const int modelID = _addedModelID++;
    _props[modelID].push_back(actor);
    add(actor);
    return modelID;
}   // end add


// public
void ModelViewer::add( const vtkProp* prop)
{
    _qviewer->add(prop);
}   // end add


// public
void ModelViewer::remove( const vtkProp* prop)
{
    _qviewer->remove(prop);
}   // end remove


// public
void ModelViewer::setLegendColours( const cv::Vec3b& col0, const cv::Vec3b& col1, int ncols)
{
    const vtkColor3ub minCol( col0[0], col0[1], col0[2]);
    const vtkColor3ub maxCol( col1[0], col1[1], col1[2]);
    _scalarLegend->setColours( ncols, minCol, maxCol);
}   // end setLegendColours


// public
void ModelViewer::setLegendColours( const QColor& col0, const QColor& col1, int ncols)
{
    const cv::Vec3b cvcol0( col0.red(), col0.green(), col0.blue());
    const cv::Vec3b cvcol1( col1.red(), col1.green(), col1.blue());
    setLegendColours( cvcol0, cvcol1, ncols);
}   // end setLegendColours


// private
int ModelViewer::addPointsActor( vtkSmartPointer<vtkActor> actor, const ModelViewer::VisOptions& vo)
{
    actor->GetProperty()->SetRepresentationToPoints();
    actor->GetProperty()->SetRenderPointsAsSpheres(true);
    actor->GetProperty()->SetPointSize( vo.pointSize);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);

    const int modelID = _addedModelID++;
    _props[modelID].push_back(actor);
    add(actor);
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
    _props[modelID].push_back(actor);
    add(actor);
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
    _props[modelID].push_back(actor);
    add(actor);
    return modelID;
}   // end addLinePairs


bool ModelViewer::remove( int modelID)
{
    if ( !_props.count(modelID))
        return false;

    const std::vector<vtkProp*>& props = _props.at(modelID);
    BOOST_FOREACH ( vtkProp* prop, props)
        remove( prop);
    _props.erase(modelID);
    return true;
}   // end remove


void ModelViewer::removeAll()
{
    typedef std::pair<int, std::vector<vtkProp*> > PPair;
    BOOST_FOREACH ( PPair ppair, _props)
        remove( ppair.first);
}   // end removeAll


CameraParams ModelViewer::getCamera() const
{
    CameraParams cp;
    _qviewer->getCamera( cp);
    return cp;
}   // end getCamera


void ModelViewer::setCamera( const CameraParams& cp)
{
    _qviewer->setCamera( cp);
}   // end setCamera


size_t ModelViewer::getWidth() const
{
    return _qviewer->getWidth();
}   // end getWidth


size_t ModelViewer::getHeight() const
{
    return _qviewer->getHeight();
}   // end getHeight


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
    RVTK::ImageGrabber imgGrabber( _qviewer->GetRenderWindow());
    imgGrabber.update();
    RFeatures::showImage( imgGrabber.getColourMap(), "SNAPSHOT", waitForInput);
}   // end showSnapshot


bool ModelViewer::saveSnapshot() const
{
    return QTools::saveImage( _qviewer->getColourImg());
}   // end saveSnapshot

