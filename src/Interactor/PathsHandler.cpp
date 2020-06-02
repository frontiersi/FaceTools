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
using FaceTools::Vec2f;
using FaceTools::Vec3f;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


PathsHandler::Ptr PathsHandler::create() { return Ptr( new PathsHandler);}

// private
PathsHandler::PathsHandler() : _handle(nullptr), _dragging(false), _initPlacement(false), _lmkVis(nullptr) {}


void PathsHandler::refreshState()
{
    const FV *fv = MS::selectedView();
    if ( _handle && fv)
    {
        const int pid = _handle->pathId();
        FM *fm = fv->data();
        fm->lockForRead();

        QString handleCaption;
        const Path& path = fm->currentAssessment()->paths().path( pid);
        if ( !path.name().empty())
            handleCaption = QString::fromStdString( path.name()) + "\n";
        if ( _handle->handleId() <= 1)
            handleCaption += QString("%1 ").arg( path.euclideanDistance(), 4, 'f', 1) + FM::LENGTH_UNITS;
        else
            handleCaption += QString("%1 ").arg( path.depth(), 4, 'f', 1) + FM::LENGTH_UNITS;
        _handle->setCaption( handleCaption);
        _vis.setText( fm, pid, int( fv->viewer()->getWidth()) - 10, 10);    // Set display caption at bottom right

        fm->unlock();
    }   // end if
    setEnabled( fv && fv->isApplied(&_vis));
}   // end refreshState


void PathsHandler::doEnterProp( FV* fv, const vtkProp* p)
{
    if ( fv == MS::selectedView())
    {
        PathView::Handle* h = _vis.pathHandle( fv, p);
        if ( !_dragging)
        {
            _handle = h;
            if ( _handle)
                _enterPath();
        }   // end if
    }   // end if
}   // end doEnterProp


void PathsHandler::doLeaveProp( FV* fv, const vtkProp* p)
{
    assert(fv);
    if ( fv == MS::selectedView() && _handle && (!_dragging || _initPlacement))
    {
        const PathView::Handle* h = _vis.pathHandle( fv, p);
        if ( h == _handle)
        {
            if ( _initPlacement)
                leftButtonUp();
            leavePath();
        }   // end if
    }   // end if
}   // end doLeaveProp


void PathsHandler::addPath( const FV *fv, int pid)
{
    _vis.addPath( fv->data(), pid);  // Adds PathView to all views of the model data
    _handle = _vis.pathHandle0( fv, pid);
    assert(_handle);
    _enterPath();
    _dragging = true;
    _initPlacement = true;
    const_cast<FV*>(fv)->apply( &_vis, nullptr);
    leftDrag();
}   // end addPath


bool PathsHandler::leftButtonDown()
{
    if ( _dragging)
        leftButtonUp();
    if ( _handle != nullptr)
    {
        emit onStartedDrag(_handle);
        _dragging = true;
    }   // end if
    return _dragging;
}   // end leftButtonDown


bool PathsHandler::leftButtonUp()
{
    bool swallowed = false;
    if ( _dragging)
    {
        assert(_handle);
        emit onFinishedDrag(_handle);
        if ( MS::cursorProp() != _handle->prop())
            leavePath();
        _dragging = false;
        _initPlacement = false;
        swallowed = true;
    }   // end if
    return swallowed;
}   // end leftButtonUp


bool PathsHandler::mouseMove() { return leftDrag();}


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


bool PathsHandler::leftDrag()
{
    if ( !_dragging)
        return false;

    assert( _handle);
    const int hid = _handle->handleId();
    const FV *fv = MS::selectedView();
    const FMV *fmv = fv->viewer();
    const QPoint mc = fmv->mouseCoords();
    FM *fm = fv->data();
    fm->lockForWrite();
    Path& path = fm->currentAssessment()->paths().path( _handle->pathId());

    if ( hid != 2)
    {
        Vec3f v; // Get the new handle position on the face
        if ( !fv->projectToSurface( mc, v))
            v = path.handle( hid);
        // Calculate the orientation vector for the path
        const r3d::CameraParams cp = fmv->camera();
        const Vec3f u = cp.pos() - cp.focus();
        const float unorm = u.norm();

        // Check if can snap to endpoint of any other path. If don't snap and landmarks are visible, snap to a landmark.
        // Snap range scales with view distance for an apparently fixed distance no matter where the camera is positioned.
        const float sqRange = powf( 0.015f*unorm, 2);
        const Vec3f inv = v;
        v = snapToPathHandle( _handle->pathId(), fm->currentAssessment()->paths(), v, sqRange);
        if ( _lmkVis && _lmkVis->isVisible(fv) && inv == v)
            v = fm->currentLandmarks().snapToVisible( v, sqRange);

        path.setOrientation( u/unorm);
        path.setHandle( hid, v);    // Handle position (transformed)
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

    path.update( fm);
    fm->setMetaSaved(false);

    _vis.updatePath( fm, _handle->pathId());
    fm->unlock();
    MS::updateRender();

    _enterPath();
    return true;
}   // end leftDrag


void PathsHandler::_enterPath()
{
    assert( _handle);
    refreshState();
    _handle->showCaption(true);
    _vis.showText( MS::selectedView());
    MS::showStatus( posString( "Handle", _handle->viewPos()), 5000);
    MS::setCursor( Qt::CursorShape::CrossCursor);
}   // end _enterPath


void PathsHandler::leavePath()
{
    if ( _handle)
        _handle->showCaption(false);
    _vis.showText(nullptr);
    _handle = nullptr;
    MS::restoreCursor();
}   // end leavePath
