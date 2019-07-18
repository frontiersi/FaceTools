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
using FaceTools::Interactor::MouseHandler;
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

void ModelViewer::updateRender() { _qviewer->updateRender();}


// public
ModelViewer::ModelViewer( QWidget* parent, bool floodFill)
    : QWidget(parent), _qviewer( nullptr), _floodLightsEnabled(floodFill)
{
    _qviewer = new QTools::VtkActorViewer(this);
    enableFloodLights( _floodLightsEnabled);

    setLayout(new QVBoxLayout);
    layout()->setContentsMargins( QMargins(0,0,0,0));
    layout()->addWidget(_qviewer);
}   // end ctor


// public
ModelViewer::~ModelViewer() { delete _qviewer;}


// protected
bool ModelViewer::attach( MVI* v) { return _qviewer->attach( v);}
bool ModelViewer::detach( MVI* v) { return _qviewer->detach( v);}
bool ModelViewer::attach( MouseHandler* v) { return _qviewer->attach( v);}
bool ModelViewer::detach( MouseHandler* v) { return _qviewer->detach( v);}


// public
void ModelViewer::setSize( const cv::Size& sz) { _qviewer->setSize( static_cast<size_t>(sz.width), static_cast<size_t>(sz.height));}
void ModelViewer::show() { _qviewer->show();}
void ModelViewer::hide() { _qviewer->hide();}
cv::Point2f ModelViewer::projectProp( const cv::Vec3f& v) const { return _qviewer->projectToDisplayProportion( v);}
cv::Point ModelViewer::project( const cv::Vec3f& v) const { return _qviewer->projectToDisplay( v);}
cv::Vec3f ModelViewer::project( const cv::Point2f& p) const { return _qviewer->pickWorldPosition( p);}
cv::Vec3f ModelViewer::project( const cv::Point& p) const { return _qviewer->pickWorldPosition( p);}
void ModelViewer::setCursor( QCursor cursor) { _qviewer->setCursor(cursor);}

void ModelViewer::setBackgroundColour( const QColor& c)
{
    vtkRenderer* ren = _qviewer->getRenderer();
    ren->SetBackground( c.redF(), c.greenF(), c.blueF());
}   // end setBackgroundColour


QColor ModelViewer::backgroundColour() const
{
    vtkRenderer* ren = _qviewer->getRenderer();
    QColor c;
    c.setRedF( ren->GetBackground()[0]);
    c.setGreenF( ren->GetBackground()[1]);
    c.setBlueF( ren->GetBackground()[2]);
    return c;
}   // end backgroundColour


void ModelViewer::add( vtkProp* prop) { if ( prop) _qviewer->add(prop);}
void ModelViewer::remove( vtkProp* prop) { if ( prop) _qviewer->remove(prop);}
void ModelViewer::clear() { _qviewer->clear();}


void ModelViewer::setCamera( const CameraParams& cp)
{
    _qviewer->setCamera( cp);
    refreshClippingPlanes();
}   // end setCamera


void ModelViewer::refreshClippingPlanes()
{
    vtkCamera* vcamera = getRenderer()->GetActiveCamera();
    double cmin, cmax;  // Get the min and max clipping ranges
    vcamera->GetClippingRange( cmin, cmax);
    const CameraParams cp = camera();
    /*
    std::cerr << "Clipping plane range min --> max: " << cmin << " --> " << cmax << std::endl;
    std::cerr << "  Camera position:  " << cp.pos << std::endl;
    std::cerr << "  Camera focus:     " << cp.focus << std::endl;
    */
    const double pfdelta = cv::norm(cp.focus - cp.pos);
    //std::cerr << "  Position - Focus: " << pfdelta << std::endl;
    // If the distance between the camera position and the focus is less than 2% the
    // distance to the near clipping plane, then make the near clipping plane closer.
    cmin = 0.01 * cmax;
    const double ctol =  2*cmin > pfdelta ? 0.00001 : 0.01;
    getRenderer()->SetNearClippingPlaneTolerance(ctol);
    getRenderer()->ResetCameraClippingRange();
    updateRender();
}   // end refreshClippingPlanes


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
    const cv::Point preal = FaceTools::fromProportion( p, cv::Size2i( static_cast<int>(getWidth()),
                                                                      static_cast<int>(getHeight())));
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
    const cv::Point p = FaceTools::fromProportion( pf, cv::Size2i( static_cast<int>(getWidth()),
                                                                   static_cast<int>(getHeight())));
    return calcSurfacePosition( prop, p, worldPos);
}   // end calcSurfacePosition


// public
CameraParams ModelViewer::camera() const { return _qviewer->camera();}


void ModelViewer::resetDefaultCamera( float camRng)
{
    // Set initial camera params
    CameraParams cp = camera();
    cp.fov = 30;
    cp.up = cv::Vec3f(0,1,0);
    cp.focus = cv::Vec3f(0,0,0);
    cp.pos = cp.focus;
    cp.pos[2] += camRng; // Set the camera position to be directly infront of the focus
    setCamera(cp);
}   // end resetDefaultCamera


double ModelViewer::cameraDistance() const
{
    CameraParams cp = camera();
    return cv::norm( cp.pos - cp.focus);
}   // end cameraDistance


void ModelViewer::setCamera( const cv::Vec3f& focus, const cv::Vec3f& nvec, const cv::Vec3f& uvec, double camRng)
{
    const cv::Vec3d pos = focus + camRng*nvec;
    const CameraParams cp( pos, focus, uvec);
    setCamera( cp);
}   // end setCamera


void ModelViewer::setCamera( const cv::Vec3f& foc, const cv::Vec3f& pos)
{
    CameraParams cp = camera();
    cp.focus = foc;
    cp.pos = pos;
    // Ensure up vector remains orthogonal to position - focus
    cv::Vec3f cvec, uvec;
    cv::normalize( cp.pos - cp.focus, cvec);
    cv::normalize( cp.up, uvec);
    const cv::Vec3f rvec = cvec.cross(uvec);
    cp.up = rvec.cross(cvec);
    setCamera( cp);
}   // end setCamera


void ModelViewer::setCameraFocus(const cv::Vec3f &foc)
{
    setCamera( foc, camera().pos);
}   // end setCameraFocus


void ModelViewer::setCameraPosition(const cv::Vec3f &pos)
{
    setCamera( camera().focus, pos);
}   // end setCameraPosition


void ModelViewer::fitCamera( double radius)
{
    // Set new field of view based on params.faceCropRadius unless custom field of view set
    CameraParams cp = camera();
    const double crng = cv::norm(cp.focus - cp.pos);
    cp.fov = atan2( radius, crng) * 360.0/CV_PI;
    setCamera(cp);
}   // end fitCamera


cv::Mat_<cv::Vec3b> ModelViewer::grabImage() const
{
    return RVTK::ImageGrabber( _qviewer->GetRenderWindow()).colour();
}   // end grabImage
