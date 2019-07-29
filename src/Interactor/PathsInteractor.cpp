/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <PathsInteractor.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::FaceViewInteractor;
using FaceTools::Interactor::PathsInteractor;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::Vis::PathView;
using FaceTools::Vis::FV;
using FaceTools::Path;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


PathsInteractor::Ptr PathsInteractor::create( PathSetVisualisation& vis)
{
    return Ptr( new PathsInteractor( vis));
}   // end create


PathsInteractor::PathsInteractor( PathSetVisualisation& vis)
    : _vis(vis), _drag(false), _handle(nullptr) {}


void PathsInteractor::enterProp( FV* fv, const vtkProp* p)
{
    if ( fv == MS::selectedView())
    {
        PathView::Handle* h = _vis.pathHandle( fv, p);
        if ( h && !_drag)
            enterPath( fv, h);
    }   // end if
}   // end enterProp


void PathsInteractor::leaveProp( FV* fv, const vtkProp*)
{
    if ( fv == MS::selectedView())
    {
        if ( _handle && !_drag)
            leavePath();
    }   // end if
}   // end leaveProp


void PathsInteractor::setPathDrag( const FV* fv, int pid)
{
    _drag = true;
    enterPath( fv, _vis.pathHandle0( fv, pid));
    cv::Vec3f pos;
    if ( fv->projectToSurface( MS::mousePos(), pos))
        movePath( fv, pos);
}   // end setPathDrag


bool PathsInteractor::leftButtonDown()
{
    if ( _drag)
        leftButtonUp();
    if ( _handle)
    {
        emit onStartedDrag(_handle);
        _drag = true;
    }   // end if
    return _drag;
}   // end leftButtonDown


bool PathsInteractor::leftButtonUp()
{
    bool swallowed = false;
    if ( _drag)
    {
        emit onFinishedDrag(_handle);
        if ( prop() != _handle->prop())
            leavePath();
        _drag = false;
        swallowed = true;
    }   // end if
    return swallowed;
}   // end leftButtonUp


bool PathsInteractor::leftDrag()
{
    if ( !view() || !_drag)
        return FVI::leftDrag();

    cv::Vec3f pos; // Get the position on the surface of the actor
    if ( !view()->projectToSurface( MS::mousePos(), pos))
        return FVI::leftDrag();

    movePath( view(), pos);
    return FVI::leftDrag();
}   // end leftDrag


bool PathsInteractor::mouseMove()
{
    if ( !view() || !_drag)
        return FVI::mouseMove();

    cv::Vec3f pos; // Get the position on the surface of the actor
    if ( !view()->projectToSurface( MS::mousePos(), pos))
        return FVI::mouseMove();

    movePath( view(), pos);
    return FVI::mouseMove();
}   // end mouseMove


void PathsInteractor::enterPath( const FV* fv, PathView::Handle* h)
{
    assert(h);
    _handle = h;
    setCaption( fv, _handle->pathId());
    h->showCaption(true);
    _vis.showText( fv->data());
    emit onEnterPath(h);
    MS::setCursor(Qt::CursorShape::CrossCursor);
}   // end enterPath


void PathsInteractor::leavePath()
{
    _handle->showCaption(false);
    _vis.showText(nullptr);
    emit onLeavePath(_handle);
    _handle = nullptr;
    _drag = false;
    MS::restoreCursor();
}   // end leavePath


void PathsInteractor::movePath( const FV* fv, const cv::Vec3f& pos)
{
    assert( _handle);
    int pid = _handle->pathId();
    FM* fm = fv->data();
    fm->lockForWrite();
    fm->setPathPosition( pid, _handle->handleId(), pos);
    _vis.movePath( fm, pid);
    fm->unlock();
    setCaption( fv, pid);
    MS::setCursor(Qt::CursorShape::CrossCursor);
}   // end movePath


void PathsInteractor::setCaption( const FV* fv, int pid)
{
    const FM* fm = fv->data();
    fm->lockForRead();
    const Path* path = fm->currentAssessment()->paths().path( pid);
    if ( _handle && _handle->pathId() == pid)
        _handle->setCaption( QString("%1 ").arg(path->elen, 4, 'f', 1) + FM::LENGTH_UNITS);
    _vis.setText( fm, pid, int( fv->viewer()->getWidth()) - 10, 10);    // Set display caption at bottom right
    fm->unlock();
}   // end setCaption
