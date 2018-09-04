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


// public
PathSetInteractor::PathSetInteractor( MEEI* meei, PathSetVisualisation* vis, QStatusBar *sbar)
    : ModelViewerInteractor( nullptr, sbar), _meei(meei), _vis(vis), _drag(-1), _hover(-1), _handle(0)
{
    connect( meei, &MEEI::onEnterProp, [this](FV* fv, const vtkProp* p){ this->doOnEnterHandle(fv, _vis->pathHandle(fv, p));});
    connect( meei, &MEEI::onLeaveProp, [this](FV* fv, const vtkProp* p){ this->doOnLeaveHandle(fv, _vis->pathHandle(fv, p));});
    setEnabled(false);
}   // end ctor


// public slot
void PathSetInteractor::doOnEnterHandle( const FV* fv, const PathView::Handle* h)
{
    if ( !h)
        return;

    _hover = h->pathId();
    _handle = h->handleId();
    assert(_handle == 0 || _handle == 1);
    //fv->data()->lockForRead();
    setCaptionInfo( fv->data(), _hover);
    setCaptionAttachPoint( fv->data(), _hover);
    //fv->data()->unlock();
    _vis->setCaptionsVisible( true);
    showStatus( s_msg0, 10000);
    fv->data()->updateRenderers();
}   // end doOnEnterHandle


// public slot
void PathSetInteractor::doOnLeaveHandle( const FV* fv, const PathView::Handle* h)
{
    _hover = -1;
    if ( h && _drag < 0)
    {
        _vis->setCaptionsVisible( false);
        fv->data()->updateRenderers();
        showStatus( s_msg1, 10000);
    }   // end if
}   // end doOnLeaveHandle


// public
void PathSetInteractor::setPathDrag( int pid)
{
    _drag = pid;
    viewer()->setCursor(Qt::CrossCursor);
}   // end setPathDrag


// public
void PathSetInteractor::setCaptionInfo( const FM* fm, int pid)
{
    const Path* path = fm->paths()->path(pid);
    assert(path);
    // Set display caption at bottom right
    _vis->setCaptions( path->name, path->elen, path->psum, (int)(viewer()->getWidth()) - 10, 10);
}   // end setCaptionInfo


// public
void PathSetInteractor::setCaptionAttachPoint( const FM* fm, int pid)
{
    const Path* path = fm->paths()->path(pid);
    assert(path);
    assert(!path->vtxs.empty());
    const cv::Vec3f attachPoint = path->vtxs[path->vtxs.size()/2];  // Halfway along path
    _vis->setCaptionAttachPoint( attachPoint);
}   // end setCaptionAttachPoint


// public
bool PathSetInteractor::moveDragHandle( const cv::Vec3f& hpos)
{
    if ( _drag < 0)
        return false;

    FV *fv = hoverModel();
    if ( !fv)
        return false;

    FM* fm = fv->data();
    //fm->lockForWrite();
    Path* path = fm->paths()->path(_drag);
    assert(path);
    assert(path->vtxs.size() >= 2);

    if ( _handle == 0)
        *path->vtxs.begin() = hpos;
    else
        *path->vtxs.rbegin() = hpos;
    path->recalculate( fm->kdtree());

    // Update visualisation over all associated FVs
    _vis->updatePath( fm, _drag);
    setCaptionInfo( fm, _drag);
    setCaptionAttachPoint( fm, _drag);

    //fm->unlock();
    return true;
}   // end moveDragHandle


bool PathSetInteractor::leftButtonDown( const QPoint& p)
{
    leftButtonUp(p);
    if ( _hover >= 0)
        setPathDrag( _hover);
    return _drag >= 0;
}   // end leftButtonDown


bool PathSetInteractor::leftButtonUp( const QPoint& p)
{
    viewer()->setCursor(Qt::ArrowCursor);
    _drag = -1;
    FV *fv = hoverModel();
    if ( !fv)
        return false;
    emit onChangedData(fv);
    return false;
}   // end leftButtonUp


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


bool PathSetInteractor::mouseMove( const QPoint& p) { return leftDrag(p);}


void PathSetInteractor::onEnabledStateChanged( bool v)
{
    if ( v)
        showStatus( s_msg1, 10000);
    else
        clearStatus();
}   // end onEnabledStateChanged
