/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <r3dvis/VtkTools.h>
#include <r3dvis/VtkActorCreator.h>
#include <QTools/QImageTools.h>
#include <vtkMapper.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <algorithm>
#include <cassert>
#include <QVBoxLayout>
using FaceTools::ModelViewer;
using r3d::CameraParams;
using FaceTools::Interactor::ViewerNotifier;
using FaceTools::Interactor::MouseHandler;
using QTools::VtkViewerInteractorManager;
using QTools::InteractionMode;
using FaceTools::Vec3f;


void ModelViewer::enableFloodLights( bool enable)
{
    std::vector<r3dvis::Light> lights;
    if ( enable)
        r3dvis::createBoxLights( 100000, lights, true);
    else
        lights.push_back( r3dvis::Light()); // Default r3dvis::Light is a bright white headlight
    _floodLightsEnabled = enable;
    _qviewer->setLights( lights);
}   // end enableFloodLights


bool ModelViewer::floodLightsEnabled() const { return _floodLightsEnabled;}

void ModelViewer::updateRender() { _qviewer->updateRender();}


ModelViewer::ModelViewer( QWidget* parent, bool floodFill)
    : QWidget(parent), _qviewer( nullptr), _floodLightsEnabled(floodFill)
{
    _qviewer = new QTools::VtkActorViewer( this);
    enableFloodLights( _floodLightsEnabled);

    setLayout(new QVBoxLayout);
    layout()->setContentsMargins( QMargins(0,0,0,0));
    layout()->addWidget(_qviewer);
}   // end ctor


ModelViewer::~ModelViewer() { delete _qviewer;}


// protected
bool ModelViewer::attach( ViewerNotifier* v) { return _qviewer->attach( v);}
bool ModelViewer::detach( ViewerNotifier* v) { return _qviewer->detach( v);}
bool ModelViewer::attach( MouseHandler* v) { return _qviewer->attach( v);}
bool ModelViewer::detach( MouseHandler* v) { return _qviewer->detach( v);}

void ModelViewer::resizeEvent( QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    emit onResized();
}   // end resizeEvent


void ModelViewer::setSize( const cv::Size& sz) { _qviewer->setSize( static_cast<size_t>(sz.width), static_cast<size_t>(sz.height));}
void ModelViewer::show() { _qviewer->show();}
void ModelViewer::hide() { _qviewer->hide();}
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


size_t ModelViewer::getWidth() const { return _qviewer->getWidth();}
size_t ModelViewer::getHeight() const { return _qviewer->getHeight();}


Vec3f ModelViewer::project( const QPoint &q) const { return _qviewer->pickPosition(q);}
QPoint ModelViewer::project( const Vec3f &v) const { return _qviewer->projectToDisplayPoint(v);}


const vtkProp* ModelViewer::getPointedAt( const cv::Point& p) const { return _qviewer->pickActor( p);}
const vtkProp* ModelViewer::getPointedAt( const QPoint& p) const { return _qviewer->pickActor( p);}

const vtkProp* ModelViewer::getPointedAt( const cv::Point2f& p) const
{
    const cv::Point preal = FaceTools::fromProportion( p, cv::Size2i( static_cast<int>(getWidth()),
                                                                      static_cast<int>(getHeight())));
    return getPointedAt(preal);
}   // end getPointedAt


bool ModelViewer::getPointedAt( const QPoint& q, const vtkActor* actor) const { return _qviewer->pointedAt( q, actor);}


bool ModelViewer::calcSurfacePosition( const vtkProp *prop, const QPoint& q, Vec3f& worldPos) const
{
    const cv::Point p(q.x(), q.y());
    return calcSurfacePosition( prop, p, worldPos);
}   // end calcSurfacePosition


bool ModelViewer::calcSurfacePosition( const vtkProp *prop, const cv::Point& p, Vec3f& worldPos) const
{
    return _qviewer->pickPosition( prop, p, worldPos);
    /*
    if ( !prop)
        return false;
    r3dvis::PointPlacer::Ptr pplacer = r3dvis::PointPlacer::create( _qviewer->getRenderer());
    pplacer->set(prop);
    const bool found = pplacer->calcSurfacePosition( p.x, p.y, &worldPos[0], r3dvis::TOP_LEFT_DISPLAY_ORIGIN);
    return found;
    */
}   // end calcSurfacePosition


bool ModelViewer::calcSurfacePosition( const vtkProp *prop, const cv::Point2f& pf, Vec3f& worldPos) const
{
    const cv::Point p = FaceTools::fromProportion( pf, cv::Size2i( static_cast<int>(getWidth()),
                                                                   static_cast<int>(getHeight())));
    return calcSurfacePosition( prop, p, worldPos);
}   // end calcSurfacePosition


CameraParams ModelViewer::camera() const { return _qviewer->camera();}

void ModelViewer::setCamera( const CameraParams& cp)
{
    _qviewer->setCamera( cp);
    _qviewer->refreshClippingPlanes();
}   // end setCamera

void ModelViewer::refreshClippingPlanes() { _qviewer->refreshClippingPlanes();}

void ModelViewer::resetDefaultCamera( float camRng) { setCamera( CameraParams( Vec3f(0,0,camRng)));}

void ModelViewer::setCamera( const Vec3f& foc, const Vec3f& pos)
{
    CameraParams cp = camera();
    cp.set( foc, pos);
    setCamera( cp);
}   // end setCamera

void ModelViewer::setCameraFocus( const Vec3f &foc) { setCamera( foc, camera().pos());}

Vec3f ModelViewer::cameraFocus() const { return camera().focus();}

void ModelViewer::setCameraPosition( const Vec3f &pos) { setCamera( camera().focus(), pos);}

Vec3f ModelViewer::cameraPosition() const { return camera().pos();}


float ModelViewer::snapRange( float p) const
{
    const CameraParams cp = camera();
    const Vec3f u = cp.pos() - cp.focus();
    return p * u.norm();
}   // end snapRange


void ModelViewer::fitCamera( float r)
{
    CameraParams cp = camera();
    cp.setViewRadius(r);
    setCamera(cp);
}   // end fitCamera


cv::Mat_<cv::Vec3b> ModelViewer::grabImage() { return _qviewer->getColourImg();}


void ModelViewer::setParallelProjection( bool enable)
{
    if ( enable != parallelProjection())
    {
        CameraParams cp = camera();
        vtkCamera* cam = _qviewer->getRenderer()->GetActiveCamera();
        cam->SetParallelProjection( enable);
        if ( enable)
            cam->SetParallelScale( cp.distance() * tanf(cp.fovRads()/2));
        else
        {
            const float wh = float(cam->GetParallelScale());
            cp.setPositionFromFocus( wh / tanf(cp.fovRads()/2));
        }   // end else
        setCamera(cp);
    }   // end if
}   // end setParallelProjection


bool ModelViewer::parallelProjection() const { return _qviewer->getRenderer()->GetActiveCamera()->GetParallelProjection();}
