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

#include <PathSetInteractor.h>
#include <ModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::Vis::PathView;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Path;


const QString PathSetInteractor::s_msg0(
        QObject::tr("Reposition path handle with left-click and drag; right click to remove/rename the path."));
const QString PathSetInteractor::s_msg1(
        QObject::tr("Right click on the model and select \"Add Path\" to place the end-point for a new path."));


PathSetInteractor::PathSetInteractor( MEEI* meei, PathSetVisualisation* vis, QStatusBar *sbar)
    : ModelViewerInteractor( nullptr, sbar), _meei(meei), _vis(vis), _drag(false), _handle(nullptr), _view(nullptr)
{
    connect( meei, &MEEI::onEnterProp, this, &PathSetInteractor::doOnEnterHandle);
    connect( meei, &MEEI::onLeaveProp, this, &PathSetInteractor::doOnLeaveHandle);
    setEnabled(false);
}   // end ctor


void PathSetInteractor::doOnEnterHandle( const FV* fv, const vtkProp* p)
{
    PathView::Handle* h = _vis->pathHandle(fv, p);
    if ( !h || _drag)
        return;

    _handle = h;
    FM* fm = fv->data();

    setCaptionInfo( fm, _handle->pathId());
    _handle->showCaption(true);
    _vis->showText(fm);
    viewer()->setCursor(Qt::CrossCursor);
    showStatus( s_msg0, 10000);
    fm->updateRenderers();
}   // end doOnEnterHandle


void PathSetInteractor::doOnLeaveHandle( const FV* fv)
{
    if ( !_handle || _drag)
        return;

    //if ( _handle && _handle->prop() == p)
    if ( _handle)
    {
        _handle->showCaption(false);
        viewer()->setCursor(Qt::ArrowCursor);
        _vis->showText(nullptr);
        showStatus( s_msg1, 10000);
        fv->data()->updateRenderers();
    }   // end if
    _handle = nullptr;
}   // end doOnLeaveHandle


void PathSetInteractor::setCaptionInfo( const FM* fm, int pid)
{
    const Path* path = fm->paths()->path(pid);
    assert(path);
    assert(_handle);
    _handle->setCaption( QString("%1 ").arg(path->elen, 4, 'f', 1) + FM::LENGTH_UNITS);
    _vis->setText( fm, pid, int(viewer()->getWidth()) - 10, 10);    // Set display caption at bottom right
}   // end setCaptionInfo


bool PathSetInteractor::moveDragHandle( const cv::Vec3f& hpos)
{
    if ( !_drag)
        return false;

    FV *fv = hoverModel();
    if ( !fv)
        return false;

    FM* fm = fv->data();
    //fm->lockForWrite();
    assert(_handle);
    Path* path = fm->paths()->path(_handle->pathId());
    assert(path->vtxs.size() >= 2);

    if ( _handle->handleId() == 0)
        *path->vtxs.begin() = hpos;
    else
        *path->vtxs.rbegin() = hpos;
    path->recalculate( fm->kdtree());

    // Update visualisation over all associated FVs
    _vis->updatePath( fm, _handle->pathId());
    setCaptionInfo( fm, _handle->pathId());

    //fm->unlock();
    return true;
}   // end moveDragHandle


void PathSetInteractor::setPathDrag( int pid)
{
    _drag = true;
    _view = hoverModel();
    _vis->showText(_view->data());
    _handle = _vis->pathHandle0(_view, pid);
    _view->data()->updateRenderers();
}   // end setPathDrag


// private
bool PathSetInteractor::leftButtonDown( const QPoint& p)
{
    if ( _drag)
        leftButtonUp(p);
    if ( _handle)
    {
        _drag = true;
        _view = hoverModel();
        _vis->showText(_view->data());
        _view->data()->updateRenderers();
    }   // end if
    return _drag;
}   // end leftButtonDown


// private
bool PathSetInteractor::leftButtonUp( const QPoint&)
{
    if ( _drag)
    {
        assert( _handle);
        _drag = false;
        if ( _meei->prop() != _handle->prop())
            doOnLeaveHandle( _view);
        emit onChangedData(_view);
    }   // end if
    return false;
}   // end leftButtonUp


// private
bool PathSetInteractor::leftDrag( const QPoint& p)
{
    bool rval = false;
    FV *fv = hoverModel();
    cv::Vec3f hpos; // Get the position on the surface of the actor
    if ( fv && fv->projectToSurface( p, hpos))
        rval = moveDragHandle( hpos);
    if ( rval)
        fv->data()->updateRenderers();
    return rval;
}   // end leftDrag


// private
bool PathSetInteractor::mouseMove( const QPoint& p) { return leftDrag(p);}


// private
void PathSetInteractor::onEnabledStateChanged( bool v)
{
    if ( v)
        showStatus( s_msg1, 10000);
    else
        clearStatus();
}   // end onEnabledStateChanged
