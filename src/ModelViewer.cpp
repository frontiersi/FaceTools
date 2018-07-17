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
#include <FeatureUtils.h>         // RFeatures
#include <VtkTools.h>             // RVTK
#include <VtkActorCreator.h>      // RVTK
#include <ImageGrabber.h>         // RVTK
#include <PointPlacer.h>          // RVTK
#include <QImageTools.h>          // QTools
#include <vtkMapper.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <algorithm>
#include <cassert>
#include <QVBoxLayout>
using FaceTools::ModelViewer;
using RFeatures::ObjModel;
using RFeatures::CameraParams;
using FaceTools::Interactor::MVI;
using QTools::VtkViewerInteractorManager;
using QTools::InteractionMode;


// public
void ModelViewer::enableFloodLights( bool enable)
{
    std::vector<RVTK::Light> lights;
    if ( enable)
        RVTK::createBoxLights( 600, lights, true);
    else
        lights.push_back( RVTK::Light()); // Default RVTK::Light is a bright white headlight
    _floodLightsEnabled = enable;
    _qviewer->setLights( lights);
}   // end enableFloodLights


// public
bool ModelViewer::floodLightsEnabled() const { return _floodLightsEnabled;}

void ModelViewer::showAxes( bool enable) { _axes->setVisible(enable);}
bool ModelViewer::legendShown() const { return _scalarLegend->isVisible();}
bool ModelViewer::axesShown() const { return _axes->isVisible();}
void ModelViewer::updateRender() { _qviewer->updateRender();}
void ModelViewer::showLegend( bool enable) { _scalarLegend->setVisible(enable); }


// public
ModelViewer::ModelViewer( QWidget* parent, bool floodFill)
    : QWidget(parent), _qviewer( new QTools::VtkActorViewer( NULL)), _scalarLegend(NULL), _axes(NULL),
      _floodLightsEnabled(floodFill), _addedModelID(0)
{
    _scalarLegend = new RVTK::ScalarLegend( _qviewer->GetInteractor());
    _axes = new RVTK::Axes( _qviewer->GetInteractor());
    showAxes(false);
    showLegend(false);
    enableFloodLights( _floodLightsEnabled);

    setLayout(new QVBoxLayout);
    layout()->setContentsMargins( QMargins(0,0,0,0));
    addToLayout( layout());
}   // end ctor


// public
ModelViewer::~ModelViewer()
{
    delete _axes;
    delete _scalarLegend;
    delete _qviewer;
}   // end dtor


// public
bool ModelViewer::isAttached( MVI* iface) const
{
    bool attached = _interactors.count(iface) > 0;
#ifndef NDEBUG
    if (attached)
    {
        assert( iface->viewer() == this);
        assert(_qviewer->isAttached(iface));
    }   // end if
    else
    {
        assert( iface->viewer() != this);
        assert(!_qviewer->isAttached(iface));
    }   // end else
#endif
    return attached;
}   // end iface


size_t ModelViewer::transferInteractors( ModelViewer *tv)
{
    if ( tv == this)
        return 0;
    std::unordered_set<MVI*> cinteractors = _interactors;   // Copy out since moving
    for ( MVI* mvi : cinteractors)
        mvi->setViewer(tv);
    return cinteractors.size();
}   // end transferInteractors


// protected (called by MVI from calls to MVI::setViewer())
bool ModelViewer::attach( MVI* iface)
{
    bool ok = _qviewer->attach(iface);
    if ( ok)
        _interactors.insert(iface);
    return ok;
}   // end attach


bool ModelViewer::detach( MVI* iface)
{
    bool ok = _qviewer->detach(iface);
    if ( ok)
        _interactors.erase(iface);
    return ok;
}   // end detach


// public
void ModelViewer::addToLayout( QLayout* layout) { layout->addWidget(_qviewer);}
void ModelViewer::removeFromLayout( QLayout* layout) { layout->removeWidget(_qviewer);}


// public
void ModelViewer::setSize( const cv::Size& sz) { _qviewer->setSize( sz.width, sz.height);}
void ModelViewer::show() { _qviewer->show();}
void ModelViewer::hide() { _qviewer->hide();}
cv::Point2f ModelViewer::projectProp( const cv::Vec3f& v) const { return _qviewer->projectToDisplayProportion( v);}
cv::Point ModelViewer::project( const cv::Vec3f& v) const { return _qviewer->projectToDisplay( v);}
cv::Vec3f ModelViewer::project( const cv::Point2f& p) const { return _qviewer->pickWorldPosition( p);}
cv::Vec3f ModelViewer::project( const cv::Point& p) const { return _qviewer->pickWorldPosition( p);}
const vtkProp* ModelViewer::getPointedAt( const cv::Point& p) const { return _qviewer->pickActor( p);}
void ModelViewer::setCursor( QCursor cursor) { _qviewer->setCursor(cursor);}
void ModelViewer::add( const vtkProp* prop) { _qviewer->add(prop);}
void ModelViewer::remove( const vtkProp* prop) { _qviewer->remove(prop);}
size_t ModelViewer::getNumLegendColours() const { return _scalarLegend->getNumColours();}
void ModelViewer::setCamera( const CameraParams& cp) { _qviewer->setCamera( cp);}
size_t ModelViewer::getWidth() const { return _qviewer->getWidth();}
size_t ModelViewer::getHeight() const { return _qviewer->getHeight();}
bool ModelViewer::saveSnapshot() const { return QTools::saveImage( _qviewer->getColourImg());}


// public
cv::Vec3f ModelViewer::project( const QPoint& q) const
{
    const cv::Point p( q.x(), q.y());
    return _qviewer->pickWorldPosition( p);
}   // end project


// public
const vtkProp* ModelViewer::getPointedAt( const cv::Point2f& p) const
{
    const cv::Point preal = FaceTools::fromProportion( p, cv::Size2i( (int)getWidth(), (int)getHeight()));
    return getPointedAt(preal);
}   // end getPointedAt


// public
const vtkProp* ModelViewer::getPointedAt( const QPoint& q) const
{
    const cv::Point p(q.x(), q.y());
    return _qviewer->pickActor( p);
}   // end getPointedAt


// public
bool ModelViewer::getPointedAt( const QPoint* q, const vtkActor* actor) const
{
    if ( !q || !actor)
        return false;
    std::vector<vtkActor*> pactors(1);
    pactors[0] = const_cast<vtkActor*>(actor);
    const cv::Point p(q->x(), q->y());
    return _qviewer->pickActor( p, pactors) == actor;
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
    const cv::Point p = FaceTools::fromProportion( pf, cv::Size2i( (int)getWidth(), (int)getHeight()));
    return calcSurfacePosition( prop, p, worldPos);
}   // end calcSurfacePosition


// public
int ModelViewer::add( const ObjModel* model, const ModelViewer::VisOptions& vo)
{
    // If a textured model was selected but the model has no texture, set the wireframe visualisation instead.
    ModelViewer::Visualisation mvis = vo.vis;
    if ( mvis == TEXTURE_VISUALISATION && model->getNumMaterials() == 0)
    {
        std::cerr << "ModelViewer: Texture visualisation requested but texture not present!" << std::endl;
        mvis = WIREFRAME_VISUALISATION;
    }   // end if

    RVTK::VtkActorCreator actorCreator;
    vtkSmartPointer<vtkActor> actor;
    if ( mvis == TEXTURE_VISUALISATION)
    {
        std::vector<vtkActor*> actors;
        actorCreator.generateTexturedActors( model, actors);
        assert( actors.size() == 1);    // Because all models should only have 1 texture!
        actor = actors[0];
    }   // end if
    else
    {
        actor = actorCreator.generateSurfaceActor( model);
        switch ( mvis)
        {
            case POINTS_VISUALISATION:
                actor->GetProperty()->SetRepresentationToPoints();
                break;
            case WIREFRAME_VISUALISATION:
                actor->GetProperty()->SetRepresentationToWireframe();
                break;
            case SURFACE_VISUALISATION:
                actor->GetProperty()->SetRepresentationToSurface();
                break;
        }   // end switch

        actor->GetProperty()->SetOpacity( vo.a);
        actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);
        actor->GetProperty()->SetPointSize(vo.pointSize);
        actor->GetProperty()->SetLineWidth(vo.lineWidth);
        actor->GetMapper()->SetScalarVisibility(true);
    }   // end if

    actor->GetProperty()->SetBackfaceCulling( vo.backfaceCulling);

    const int modelID = _addedModelID++;
    _props[modelID] = actor;
    add( actor);
    return modelID;
}   // end add


// public
int ModelViewer::add( vtkSmartPointer<vtkActor> actor, const std::string& ltitle, float minv, float maxv)
{
    setLegendLookup( actor->GetMapper(), ltitle, minv, maxv);
    const int modelID = _addedModelID++;
    _props[modelID] = actor;
    add(actor);
    return modelID;
}   // end add


// public
vtkProp* ModelViewer::getProp( int mid)
{
    vtkProp* prop = NULL;
    if ( _props.count(mid) > 0)
        prop = _props.at(mid);
    return prop;
}   // end getProp


// public
void ModelViewer::setLegendLookup( vtkMapper* mapper, const std::string& ltitle, float minv, float maxv)
{
    _scalarLegend->setTitle( ltitle);
    _scalarLegend->setLookupTable( mapper, minv, maxv);
}   // end setLegendLookup


// public
void ModelViewer::setLegendColours( const cv::Vec3b& col0, const cv::Vec3b& col1, size_t ncols)
{
    const vtkColor3ub minCol( col0[0], col0[1], col0[2]);
    const vtkColor3ub maxCol( col1[0], col1[1], col1[2]);
    _scalarLegend->setColours( minCol, maxCol, ncols);
}   // end setLegendColours


// public
void ModelViewer::setLegendColours( const QColor& col0, const QColor& col1, size_t ncols)
{
    const cv::Vec3b cvcol0( col0.red(), col0.green(), col0.blue());
    const cv::Vec3b cvcol1( col1.red(), col1.green(), col1.blue());
    setLegendColours( cvcol0, cvcol1, ncols);
}   // end setLegendColours


// public
void ModelViewer::setLegendColours( const cv::Vec3b& col0, const cv::Vec3b& col1, const cv::Vec3b& col2, size_t ncols0, size_t ncols1)
{
    const vtkColor3ub c0( col0[0], col0[1], col0[2]);
    const vtkColor3ub c1( col1[0], col1[1], col1[2]);
    const vtkColor3ub c2( col2[0], col2[1], col2[2]);
    _scalarLegend->setColours( c0, c1, c2, ncols0, ncols1);
}   // end setLegendColours


// public
void ModelViewer::setLegendColours( const QColor& col0, const QColor& col1, const QColor& col2, size_t ncols0, size_t ncols1)
{
    const vtkColor3ub c0( col0.red(), col0.green(), col0.blue());
    const vtkColor3ub c1( col1.red(), col1.green(), col1.blue());
    const vtkColor3ub c2( col2.red(), col2.green(), col2.blue());
    _scalarLegend->setColours( c0, c1, c2, ncols0, ncols1);
}   // end setLegendColours


// private
int ModelViewer::addPointsActor( vtkSmartPointer<vtkActor> actor, const ModelViewer::VisOptions& vo, bool asSpheres)
{
    actor->GetProperty()->SetRepresentationToPoints();
    //actor->GetProperty()->SetRenderPointsAsSpheres( asSpheres && vo.pointSize >= 2.0f);
    actor->GetProperty()->SetPointSize( vo.pointSize);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);
    actor->GetProperty()->SetOpacity( vo.a);

    const int modelID = _addedModelID++;
    _props[modelID] = actor;
    add(actor);
    return modelID;
}   // end addPointsActor


// public
int ModelViewer::addPoint( const cv::Vec3f& vpt, const ModelViewer::VisOptions& vo)
{
    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius( vo.pointSize);
    sphere->SetCenter( vpt[0], vpt[1], vpt[2]);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( sphere->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    actor->GetProperty()->SetPointSize( vo.pointSize);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);
    actor->GetProperty()->SetOpacity( vo.a);

    const int modelID = _addedModelID++;
    _props[modelID] = actor;
    add(actor);
    return modelID;
}   // end addPoint


int ModelViewer::addPoints( const std::vector<cv::Vec3f>& vpts, const ModelViewer::VisOptions& vo, bool asSpheres)
{
    return addPointsActor( RVTK::VtkActorCreator::generatePointsActor( vpts), vo, asSpheres);
}   // end addPoints


int ModelViewer::addPoints( const ObjModel* model, const ModelViewer::VisOptions& vo, bool asSpheres)
{
    return addPointsActor( RVTK::VtkActorCreator::generatePointsActor( model), vo, asSpheres);
}   // end addPoints


int ModelViewer::addPoints( const ObjModel* model, const std::unordered_set<int>& vidxs, const ModelViewer::VisOptions& vo, bool asSpheres)
{
    int i = 0;
    std::vector<cv::Vec3f> vpts(vidxs.size());
    std::for_each( std::begin(vidxs), std::end(vidxs), [&](int vidx){ vpts[i++] = model->vtx(vidx);});
    return addPoints( vpts, vo, asSpheres);
}   // end addPoints


int ModelViewer::addLine( const std::vector<cv::Vec3f>& vpts, bool joinEnds, const ModelViewer::VisOptions& vo)
{
    vtkSmartPointer<vtkActor> actor = RVTK::VtkActorCreator::generateLineActor( vpts, joinEnds);
    actor->GetProperty()->SetRepresentationToWireframe();
    //actor->GetProperty()->SetRenderPointsAsSpheres( vo.pointSize >= 2.0f);
    //actor->GetProperty()->SetRenderPointsAsSpheres( false);
    //actor->GetProperty()->SetRenderLinesAsTubes( vo.lineWidth >= 2.0f);
    actor->GetProperty()->SetRenderLinesAsTubes( false);
    actor->GetProperty()->SetPointSize( vo.pointSize);
    actor->GetProperty()->SetLineWidth( vo.lineWidth);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);
    actor->GetProperty()->SetOpacity( vo.a);

    const int modelID = _addedModelID++;
    _props[modelID] = actor;
    add(actor);
    return modelID;
}   // end addLine


int ModelViewer::addLinePairs( const std::vector<cv::Vec3f>& lps, const ModelViewer::VisOptions& vo)
{
    vtkSmartPointer<vtkActor> actor = RVTK::VtkActorCreator::generateLinePairsActor( lps);
    actor->GetProperty()->SetRepresentationToWireframe();
    //actor->GetProperty()->SetRenderPointsAsSpheres( vo.pointSize >= 2.0f);
    //actor->GetProperty()->SetRenderPointsAsSpheres( false);
    //actor->GetProperty()->SetRenderLinesAsTubes( vo.lineWidth >= 2.0f);
    actor->GetProperty()->SetRenderLinesAsTubes( false);
    actor->GetProperty()->SetPointSize( vo.pointSize);
    actor->GetProperty()->SetLineWidth( vo.lineWidth);
    actor->GetProperty()->SetColor( vo.r, vo.g, vo.b);
    actor->GetProperty()->SetOpacity( vo.a);

    const int modelID = _addedModelID++;
    _props[modelID] = actor;
    add(actor);
    return modelID;
}   // end addLinePairs


bool ModelViewer::remove( int modelID)
{
    if ( !_props.count(modelID))
        return false;

    remove( _props.at(modelID));
    _props.erase(modelID);
    return true;
}   // end remove


void ModelViewer::removeAll()
{
    std::for_each( std::begin(_props), std::end(_props), [this]( auto& pp){ this->remove(pp.second);});
    _props.clear();
}   // end removeAll


CameraParams ModelViewer::getCamera() const
{
    CameraParams cp;
    _qviewer->getCamera( cp);
    return cp;
}   // end getCamera


void ModelViewer::resetDefaultCamera( float camRng)
{
    // Set initial camera params
    CameraParams cp = getCamera();
    cp.fov = 30;
    cp.up = cv::Vec3f(0,1,0);
    cp.focus = cv::Vec3f(0,0,0);
    cp.pos = cp.focus;
    cp.pos[2] += camRng; // Set the camera position to be directly infront of the focus
    setCamera(cp);
}   // end resetDefaultCamera


float ModelViewer::cameraDistance() const
{
    CameraParams cp = getCamera();
    return cv::norm( cp.pos - cp.focus);
}   // end cameraDistance


void ModelViewer::setCamera( const cv::Vec3f& focus, const cv::Vec3f& nvec, const cv::Vec3f& uvec, float camRng)
{
    const cv::Vec3f pos = focus + camRng*nvec;
    const CameraParams cp( pos, focus, uvec);
    setCamera( cp);
}   // end setCamera


void ModelViewer::setFocus( const cv::Vec3f& focus)
{
    CameraParams cp = getCamera();
    // Set camera position to be exactly in front of focus at current position along +Z
    float ndist = cp.pos[2] - focus[2]; // Maintain distance as along +Z axis
    cp.focus = focus;
    setCamera( cp);
}   // end setFocus


void ModelViewer::fitCamera( double radius)
{
    // Set new field of view based on params.faceCropRadius unless custom field of view set
    CameraParams cp = getCamera();
    const double crng = cv::norm(cp.focus - cp.pos);
    cp.fov = atan2( radius, crng) * 360.0/CV_PI;
    setCamera(cp);
}   // end fitCamera


cv::Mat_<cv::Vec3b> ModelViewer::grabImage() const
{
    return RVTK::ImageGrabber( _qviewer->GetRenderWindow()).colour();
}   // end grabImage
