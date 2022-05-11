/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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
#include <ModelSelect.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
#include <MiscFunctions.h>
#include <cassert>
using FaceTools::Interactor::PathsHandler;
using FaceTools::Vis::PathView;
using FaceTools::Vis::FV;
using FaceTools::Path;
using FaceTools::Vec3f;
using MS = FaceTools::ModelSelect;
using LMAN = FaceTools::Landmark::LandmarksManager;


PathsHandler::Ptr PathsHandler::create() { return Ptr( new PathsHandler);}

// private
PathsHandler::PathsHandler() : _handle(nullptr), _dragging(false), _initPlacement(false) {}


void PathsHandler::refresh()
{
    const FV *fv = MS::selectedView();
    setEnabled( fv && _vis.isVisible(fv));

    // Handle possibility of closed model or deleted paths
    if ( !fv || fv->rdata()->currentPaths().empty())
        _handle = nullptr;
    else if ( isEnabled() && _handle)
        _updateCaption();
}   // end refresh


bool PathsHandler::doEnterProp()
{
    const FV *fv = MS::selectedView();
    const vtkProp *p = this->prop();
    bool swallowed = false;
    PathView::Handle* h = _vis.pathHandle( fv, p);
    if ( !_dragging && h)
    {
        _handle = h;
        swallowed = true;
        _showPathInfo();
        emit onEnterHandle( h->pathId(), h->handleId());
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


void PathsHandler::addPath( Vec3f &v)
{
    const FV *fv = MS::selectedView();
    _snapHandle( fv, v);
    FM::WPtr fm = MS::selectedModelScopedWrite();
    const int pid = fm->addPath( v);
    _dragging = true;
    _initPlacement = true;
    for ( FV *fv : fm->fvs())
        fv->apply( &_vis);
    _handle = _vis.pathHandle0( MS::selectedView(), pid);
    assert(_handle);
    _execLeftDrag();
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
        if ( pid != hpid)   // Don't allow handle snapping to the same path!
        {
            const FaceTools::Path &opath = paths.path(pid);
            checkCloser( minSqDist, nv, opath.handle0(), v, sqRange);
            checkCloser( minSqDist, nv, opath.handle1(), v, sqRange);
            checkCloser( minSqDist, nv, opath.depthHandle(), v, sqRange);
        }   // end if
    }   // end for
    return nv;
}   // end snapToPathHandle

}   // end namespace


// Needed only when initial placement happening
bool PathsHandler::doMouseMove()
{
    bool swallowed = false;
    if ( _initPlacement)
    {
        assert(_handle);
        swallowed = doLeftDrag();
        // Also update position of depth handle to be midway between handle endpoints
        const FV *fv = MS::selectedView();
        FM::WPtr fm = MS::selectedModelScopedWrite();
        const int pid = _handle->pathId();
        Path& path = fm->currentPaths().path( pid);
        Vec3f v = (path.handle0() + path.handle1())*0.5f;
        const QPoint qp = fv->viewer()->project( v);
        if ( !fv->projectToSurface( qp, v))
            v = path.handle0();
        path.setDepthHandle( v);

        path.updateMeasures( fm->inverseTransformMatrix().block<3,3>(0,0));
        fm->setMetaSaved( false);
        _vis.updatePath( *fm, pid);
        _showPathInfo();
    }   // end if
    return swallowed;
}   // end doMouseMove


bool PathsHandler::doLeftDrag()
{
    const bool rv = _execLeftDrag();
    MS::updateRender(); // Ensure render update happens across viewers
    return rv;
}   // end doLeftDrag


void PathsHandler::_snapHandle( const FV *fv, Vec3f &v) const
{
    const FMV *fmv = fv->viewer();
    const float sqRange = powf( std::min( 3.0f, fmv->snapRange()), 2);  // Snap range max is 3.0mm
    const Vec3f inv = v;
    const FM *fm = fv->data();
    // Check if can snap to endpoint of another path. If don't snap and landmarks are visible, snap to one.
    // Snap range scales with view distance for an apparently fixed distance no matter where camera is.
    if ( _handle)
        v = snapToPathHandle( _handle->pathId(), fm->currentPaths(), v, sqRange);

    // If didn't snap to another path (inv == v), see if snap to a landmark
    const Vis::BaseVisualisation *lmkVis = &MS::handler<LandmarksHandler>()->visualisation();
    assert( lmkVis);
    if ( lmkVis->isVisible(fv) && inv == v)
    {
        v = fm->currentLandmarks().snapTo( v, sqRange);
        // Use unsnapped if the snapped position is the same as the other handles of the current path.
        if ( _handle)
        {   
            const int hid = _handle->handleId();
            const Path &path = fm->currentPaths().path( _handle->pathId());
            if ( v == path.handle( (hid+1)%3) || v == path.handle( (hid+2)%3))
                v = inv;
        }   // end if
    }   // end if
}   // end _snapHandle


bool PathsHandler::_execLeftDrag()
{
    if ( !_dragging)
        return false;

    const int hid = _handle->handleId();
    const int pid = _handle->pathId();
    FV *fv = MS::selectedView();
    const FMV *fmv = fv->viewer();
    FM *fm = fv->data();
    Path& path = fm->currentPaths().path( pid);

    Vec3f v; // Get the new handle position on the face
    if ( !fv->projectToSurface( fmv->mouseCoords(), v))
        v = path.handle( hid);

    _snapHandle( fv, v);        // Snap to other path or landmark (if visible)

    if ( hid != 2)  // Endpoints changed so redraw path using current camera vector
    {
        path.setHandle( hid, v);    // Handle position (transformed)
        const r3d::CameraParams cp = fmv->camera();
        path.setOrientation( cp.pos() - cp.focus());    // Orientation will be normalized
        path.updatePath( fm);         // Update the surface path
    }   // end if
    else
    {
        //std::cerr << "Depth handle " << v << std::endl;
        path.setDepthHandle( v);
    }   // end else

    path.updateMeasures( fm->inverseTransformMatrix().block<3,3>(0,0));
    fm->setMetaSaved( false);
    _vis.updatePath( *fm, pid);
    _showPathInfo();

    return true;
}   // end _execLeftDrag



void PathsHandler::_showPathInfo()
{
    assert( _handle);
    _updateCaption();
    _handle->showCaption(true);
    _vis.showCaption( MS::selectedView());
    MS::showStatus( posString( "Handle", _handle->viewPos()), 5000);
    MS::setCursor( Qt::CursorShape::CrossCursor);
    MS::updateRender();
}   // end _showPathInfo


namespace {

QString makeHandleCaption( const Path &path, int hid)
{
    QString handleCaption;
    QTextStream out( &handleCaption);
    if ( !path.name().isEmpty())
        out << path.name() << "\n";
    const QString units = FaceTools::FaceModel::LENGTH_UNITS;
    if ( hid <= 1)  // Endpoints
        out << QString(" %1 %2 (Direct)").arg( path.euclideanDistance(), 6, 'f', 2).arg(units);
    else            // Orange handle
        out << QString::fromWCharArray(L" %1 \u00b0").arg( path.angle(), 6, 'f', 2);
    out << QString("\n %2 %3 (Depth)").arg( path.depth(), 6, 'f', 2).arg(units);
    return handleCaption;
}   // end makeHandleCaption
}   // end namespace


void PathsHandler::_updateCaption()
{
    assert(_handle);
    const FV *fv = MS::selectedView();
    const FM *fm = fv->data();
    const int pid = _handle->pathId();
    const int hid = _handle->handleId();
    const Path& path = fm->currentPaths().path( pid);

    _handle->setCaption( makeHandleCaption( path, hid));
    _vis.updateCaption( *fm, path); // Display caption at bottom right of viewer

    // Show the given path on the other model if the other model exists
    // and has the same mask mapped as the given (selected) model does.
    // Also show on other model if same mask used?
    FM::RPtr ofm = MS::otherModelScopedRead();
    if ( ofm && ofm->hasMask() && ofm->maskHash() == fm->maskHash())
    {
        const Path npath = path.mapSrcToDst( fm, ofm.get());
        _vis.showTemporaryPath( *ofm, npath, hid, makeHandleCaption( npath, hid));
        _vis.updateCaption( *ofm, npath);
    }   // end if
}   // end _updateCaption


int PathsHandler::leavePath()
{
    int pid = -1;
    if ( _handle)
    {
        _handle->showCaption(false);
        pid = _handle->pathId();
        emit onLeaveHandle( pid, _handle->handleId());
        _vis.clearTemporaryPath();
        _vis.showCaption(nullptr);
        _handle = nullptr;
        MS::restoreCursor();
        MS::updateRender();
    }   // end if
    return pid;
}   // end leavePath

