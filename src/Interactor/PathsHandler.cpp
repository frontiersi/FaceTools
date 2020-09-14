/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Interactor/PathsHandler.h>
#include <Interactor/LandmarksHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <Action/ModelSelector.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
#include <MiscFunctions.h>
#include <cassert>
using FaceTools::Interactor::PathsHandler;
using FaceTools::Vis::PathView;
using FaceTools::Vis::FV;
using FaceTools::Path;
using FaceTools::FM;
using FaceTools::Vec3f;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


PathsHandler::Ptr PathsHandler::create() { return Ptr( new PathsHandler);}

// private
PathsHandler::PathsHandler() : _handle(nullptr), _dragging(false), _initPlacement(false), _lmkVis(nullptr) {}


void PathsHandler::postRegister()
{
    _lmkVis = &MS::handler<LandmarksHandler>()->visualisation();
    assert( _lmkVis);
}   // end postRegister


void PathsHandler::refresh()
{
    const FV *fv = MS::selectedView();
    setEnabled( fv && _vis.isVisible(fv));

    // Handle possibility of closed model or deleted paths
    if ( !fv || fv->data()->currentAssessment()->paths().empty())
        _handle = nullptr;
    else if ( isEnabled() && _handle)
    {
        const int pid = _handle->pathId();
        FM *fm = fv->data();
        fm->lockForRead();
        _updateCaption();
        fm->unlock();
    }   // end if
}   // end refresh


bool PathsHandler::doEnterProp()
{
    const FV *fv = MS::selectedView();
    const vtkProp *p = this->prop();
    bool swallowed = false;
    PathView::Handle* h = _vis.pathHandle( fv, p);
    if ( !_dragging)
    {
        _handle = h;
        if ( _handle)
        {
            swallowed = true;
            _showPathInfo();
            emit onEnterHandle( _handle->pathId(), _handle->handleId());
        }   // end if
    }   // end if
    return swallowed;
}   // end doEnterProp


bool PathsHandler::doLeaveProp()
{
    const FV *fv = MS::selectedView();
    const vtkProp *p = this->prop();
    bool swallowed = false;
    if ( _handle && (!_dragging || _initPlacement))
    {
        const PathView::Handle* h = _vis.pathHandle( fv, p);
        if ( h == _handle)
        {
            swallowed = true;
            if ( !_initPlacement)
            {
                endDragging();
                leavePath();
            }   // end if
        }   // end if
    }   // end if
    return swallowed;
}   // end doLeaveProp


void PathsHandler::addPath( int pid)
{
    const FV *fv = MS::selectedView();
    _vis.addPath( fv->data(), pid);  // Adds PathView to all views of the model data
    _handle = _vis.pathHandle0( fv, pid);
    assert(_handle);
    _dragging = true;
    _initPlacement = true;
    for ( FV *f : fv->data()->fvs())
        f->apply( &_vis, nullptr);
    doLeftDrag();
}   // end addPath


bool PathsHandler::endDragging()
{
    bool swallowed = false;
    if ( _dragging)
    {
        swallowed = true;
        assert(_handle);
        const int pid = _handle->pathId();
        const int hid = _handle->handleId();
        if ( this->prop() != _handle->prop())
            leavePath();
        _dragging = false;
        _initPlacement = false;
        emit onFinishedDrag( pid, hid);
    }   // end if
    return swallowed;
}   // end endDragging


bool PathsHandler::doLeftButtonDown()
{
    bool swallowed = endDragging();
    if ( !swallowed && _handle && this->prop() == _handle->prop())
    {
        swallowed = true;
        _dragging = true;
        emit onStartedDrag( _handle->pathId(), _handle->handleId());
    }   // end else if
    return swallowed;
}   // end doLeftButtonDown


bool PathsHandler::doLeftButtonUp() { return endDragging();}


// Needed only when initial placement happening
bool PathsHandler::doMouseMove()
{
    bool swallowed = false;
    if ( _initPlacement)
        swallowed = doLeftDrag();
    return swallowed;
}   // end doMouseMove


namespace {
void checkCloser( float &minSqDist, Vec3f &nv, const Vec3f &vm, const Vec3f &v, float snapSqDist)
{
    const float sqdist = (vm - v).squaredNorm();
    if ( sqdist < snapSqDist && sqdist < minSqDist)
    {
        minSqDist = sqdist;
        nv = vm;
    }   // end if
}   // end checkCloser

Vec3f snapToPathHandle( int hpid, const FaceTools::PathSet &paths, const Vec3f &v, float sqRange)
{
    Vec3f nv = v;
    float minSqDist = FLT_MAX;
    for ( int pid : paths.ids())
    {
        if ( pid != hpid)
        {
            const FaceTools::Path &opath = paths.path(pid);
            checkCloser( minSqDist, nv, opath.handle0(), v, sqRange);
            checkCloser( minSqDist, nv, opath.handle1(), v, sqRange);
        }   // end if
    }   // end for
    return nv;
}   // end snapToPathHandle

}   // end namespace


bool PathsHandler::doLeftDrag()
{
    if ( !_dragging)
        return false;

    const int hid = _handle->handleId();
    const FV *fv = MS::selectedView();
    const FMV *fmv = fv->viewer();
    const QPoint mc = fmv->mouseCoords();   // NB current cursor coords according to Qt - NOT the interaction coords from VTK!
    FM *fm = fv->data();
    fm->lockForWrite();
    Path& path = fm->currentAssessment()->paths().path( _handle->pathId());

    if ( hid != 2)
    {
        Vec3f v; // Get the new endpoint handle position on the face
        if ( !fv->projectToSurface( mc, v))
            v = path.handle( hid);

        // Calculate the orientation vector for the path
        const r3d::CameraParams cp = fmv->camera();

        // Check if can snap to endpoint of another path. If don't snap and landmarks are visible, snap to one.
        // Snap range scales with view distance for an apparently fixed distance no matter where camera is.
        const float sqRange = powf( fmv->snapRange(), 2);
        const Vec3f inv = v;
        v = snapToPathHandle( _handle->pathId(), fm->currentAssessment()->paths(), v, sqRange);
        if ( _lmkVis->isVisible(fv) && inv == v)
            v = fm->currentLandmarks().snapTo( v, sqRange);

        path.setOrientation( cp.pos() - cp.focus());    // Will be normalized
        path.setHandle( hid, v);    // Handle position (transformed)
        path.update( fm);
    }   // end if
    else
    {
        const QPoint h0 = fmv->project(path.handle0());
        const QPoint h1 = fmv->project(path.handle1());

        const Vec2f pathVec( h1.x() - h0.x(), h1.y() - h0.y());
        const Vec2f dvec( mc.x() - h0.x(), mc.y() - h0.y());
        const float pathMag = pathVec.norm();
        float dprop = 0.0f;
        if ( pathMag > 0.0f)
            dprop = dvec.dot(pathVec)/(pathMag*pathMag);
        path.setDepthHandle( dprop);    // Restricts to be in [0,1]
    }   // end else

    path.updateMeasures();
    fm->setMetaSaved( false);

    _vis.updatePath( fm, _handle->pathId());
    fm->unlock();

    _showPathInfo();

    MS::updateRender(); // Ensure render update happens across viewers

    return true;
}   // end doLeftDrag


void PathsHandler::_showPathInfo()
{
    assert( _handle);
    _updateCaption();
    _handle->showCaption(true);
    _vis.showCaption( MS::selectedView());
    MS::showStatus( posString( "Handle", _handle->viewPos()), 5000);
    MS::setCursor( Qt::CursorShape::CrossCursor);
}   // end _showPathInfo


void PathsHandler::_updateCaption()
{
    const FV *fv = MS::selectedView();
    assert(fv);
    const FM *fm = fv->data();
    assert(_handle);
    const int pid = _handle->pathId();
    const Path& path = fm->currentAssessment()->paths().path( pid);

    QString handleCaption;
    if ( !path.name().empty())
        handleCaption = QString::fromStdString( path.name()) + "\n";

    float val;
    QString valType;
    if ( _handle->handleId() <= 1)
    {
        val = path.euclideanDistance();
        valType = "[Direct]";
    }   // end if
    else
    {
        val = path.depth();
        valType = "[Depth]";
    }   // end else

    handleCaption += QString("%1 %2 %3").arg( val, 4, 'f', 2).arg(FM::LENGTH_UNITS).arg(valType);
    _handle->setCaption( handleCaption);

    _vis.updateCaption( fm, pid, int( fv->viewer()->getWidth()) - 10, 10);    // Display caption at bottom right
}   // end _updateCaption


int PathsHandler::leavePath()
{
    assert(_handle);
    _handle->showCaption(false);
    const int pid = _handle->pathId();
    emit onLeaveHandle( pid, _handle->handleId());
    _vis.showCaption(nullptr);
    _handle = nullptr;
    MS::restoreCursor();
    return pid;
}   // end leavePath
