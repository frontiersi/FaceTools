/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
    : QWidget(parent), _qviewer( new QTools::VtkActorViewer( nullptr)), _scalarLegend(nullptr), _axes(nullptr),
      _floodLightsEnabled(floodFill)
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
void ModelViewer::setCursor( QCursor cursor) { _qviewer->setCursor(cursor);}

void ModelViewer::add( vtkProp* prop) { if ( prop) _qviewer->add(prop);}
void ModelViewer::remove( vtkProp* prop) { if ( prop) _qviewer->remove(prop);}

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
const vtkProp* ModelViewer::getPointedAt( const cv::Point& p) const { return _qviewer->pickActor( p);}
const vtkProp* ModelViewer::getPointedAt( const QPoint& p) const { return _qviewer->pickActor( p);}

// public
const vtkProp* ModelViewer::getPointedAt( const cv::Point2f& p) const
{
    const cv::Point preal = FaceTools::fromProportion( p, cv::Size2i( (int)getWidth(), (int)getHeight()));
    return getPointedAt(preal);
}   // end getPointedAt


// public
bool ModelViewer::getPointedAt( const QPoint& q, const vtkActor* actor) const { return _qviewer->pointedAt( q, actor);}


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
void ModelViewer::setLegend( const std::string& ltitle, vtkLookupTable* table)
{
    _scalarLegend->setTitle( ltitle);
    _scalarLegend->setLookupTable( table);
}   // end setLegend


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
