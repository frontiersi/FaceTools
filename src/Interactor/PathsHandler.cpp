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

#include <Interactor/PathsHandler.h>
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
using MS = FaceTools::Action::ModelSelector;


PathsHandler::Ptr PathsHandler::create() { return Ptr( new PathsHandler());}

PathsHandler::PathsHandler() : _vis(), _drag(false), _handle(nullptr) {}

void PathsHandler::doEnterProp( FV* fv, const vtkProp* p)
{
    if ( fv == MS::selectedView())
    {
        PathView::Handle* h = _vis.pathHandle( fv, p);
        if ( h && !_drag)
            _enterPath( fv, h);
    }   // end if
}   // end doEnterProp


void PathsHandler::doLeaveProp( FV* fv, const vtkProp* p)
{
    if ( fv == MS::selectedView())
    {
        if ( (_handle != nullptr) && !_drag)
            _leavePath();
    }   // end if
}   // end doLeaveProp


void PathsHandler::addPath( const FV* fv, const cv::Vec3f& pos)
{
    FM* fm = fv->data();
    fm->lockForWrite();
    const int pid = fm->addPath( pos);    // Added point is untransformed
    fm->unlock();
    //std::cerr << "Adding path at " << pos << std::endl;
    _vis.addPath( fm, pid);
    _drag = true;
    _enterPath( fv, _vis.pathHandle0( fv, pid));
    _movePath( fv, pos);
}   // end addPath


void PathsHandler::erasePath( const FV* fv, int pid)
{
    FM* fm = fv->data();
    fm->lockForWrite();
    fm->removePath(pid);
    fm->unlock();
    _leavePath();
    _vis.erasePath( fm, pid);
}   // end erasePath


bool PathsHandler::leftButtonDown()
{
    if ( _drag)
        leftButtonUp();
    if ( _handle != nullptr)
    {
        emit onStartedDrag(_handle);
        _drag = true;
    }   // end if
    return _drag;
}   // end leftButtonDown


bool PathsHandler::leftButtonUp()
{
    bool swallowed = false;
    if ( _drag)
    {
        assert(_handle != nullptr);
        emit onFinishedDrag(_handle);
        if ( MS::cursorProp() != _handle->prop())
            _leavePath();
        _drag = false;
        swallowed = true;
    }   // end if
    return swallowed;
}   // end leftButtonUp


bool PathsHandler::leftDrag()
{
    if ( !MS::cursorView() || !_drag)
        return false;

    cv::Vec3f pos; // Get the transformed position on the surface of the actor
    if ( !MS::cursorView()->projectToSurface( MS::mousePos(), pos))
        return false;

    _movePath( MS::cursorView(), pos);
    return true;
}   // end leftDrag


bool PathsHandler::mouseMove() { return leftDrag();}


void PathsHandler::_enterPath( const FV* fv, PathView::Handle* h)
{
    assert(h);
    _handle = h;
    setCaption( fv, _handle->pathId());
    h->showCaption(true);
    _vis.showText( fv->data());
    MS::showStatus( posString( "Handle pos:", _handle->viewPos()), 5000);
    emit onEnterPath(h);
    MS::setCursor(Qt::CursorShape::CrossCursor);
    MS::updateRender();
}   // end _enterPath


void PathsHandler::_leavePath()
{
    if ( _handle != nullptr)
    {
        _handle->showCaption(false);
        emit onLeavePath(_handle);
    }   // end if
    _vis.showText(nullptr);
    _handle = nullptr;
    _drag = false;
    MS::restoreCursor();
    MS::updateRender();
}   // end _leavePath


void PathsHandler::_movePath( const FV* fv, const cv::Vec3f& pos)   // pos is the TRANSFORMED (view) position
{
    assert( _handle);
    int pid = _handle->pathId();
    FM* fm = fv->data();
    fm->lockForWrite();
    fm->setPathPosition( pid, _handle->handleId(), pos);
    _vis.movePath( fm, pid);
    fm->unlock();
    setCaption( fv, pid);
    MS::showStatus( posString( "Handle pos:", _handle->viewPos()), 5000);
    MS::setCursor(Qt::CursorShape::CrossCursor);
    MS::updateRender(); // So the moving path is updated across all viewers and not just the viewer being interacted with
}   // end _movePath


void PathsHandler::setCaption( const FV* fv, int pid)
{
    const FM* fm = fv->data();
    fm->lockForRead();
    const Path* path = fm->currentAssessment()->paths().path( pid);
    if ( (_handle != nullptr) && _handle->pathId() == pid)
        _handle->setCaption( QString("%1 ").arg(path->elen, 4, 'f', 1) + FM::LENGTH_UNITS);
    _vis.setText( fm, pid, int( fv->viewer()->getWidth()) - 10, 10);    // Set display caption at bottom right
    fm->unlock();
}   // end setCaption
