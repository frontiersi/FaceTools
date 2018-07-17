/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
using FaceTools::Interactor::FaceHoveringInteractor;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::Vis::PathView;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Path;


// public
PathSetInteractor::PathSetInteractor( FEEI* feei, PathSetVisualisation* pvis)
    : FaceHoveringInteractor( feei, pvis), _pvis(pvis), _hdrag(nullptr), _hhover(nullptr)
{
}   // end ctor


// public
int PathSetInteractor::hoverID() const
{
    int id = -1;
    if ( _hhover)
    {
        const PathView* pview = _hhover->host();
        assert(pview);
        const Path& path = pview->path();
        id = path.id;
    }   // end if
    return id;
}   // end hoverID


// public
int PathSetInteractor::addPath()
{
    FaceControl *fc = hoverModel();
    if ( !fc)
        return -1;

    QPoint p = viewer()->getMouseCoords();
    cv::Vec3f hpos; // Get the position of the new handle by projecting p onto the surface of the model.
    if ( !viewer()->calcSurfacePosition( fc->view()->surfaceActor(), p, hpos))
        return -1;

    FaceModel* fm = fc->data();
    fm->lockForWrite(); // Add path to the dataset
    int pathId = fm->paths()->addPath(hpos);
    fm->setSaved(false);

    const FaceControlSet& fcs = fm->faceControls();    // Add to the visualisations
    std::for_each( std::begin(fcs), std::end(fcs), [&](auto f){ _pvis->addPath(f,pathId);});

    fm->unlock();

    _hhover = getPathViewHandle(p);
    _pvis->setCaptionsVisible(true);
    return pathId;
}   // end addPath


// public
bool PathSetInteractor::deletePath()
{
    int pathId = hoverID();
    if ( pathId < 0)
        return false;

    FaceControl *fc = hoverModel();
    assert(fc);

    FaceModel* fm = fc->data();
    fm->lockForWrite();
    fm->paths()->removePath(pathId);
    fm->setSaved(false);

    const FaceControlSet& fcs = fm->faceControls();    // Remove from the visualisations
    std::for_each( std::begin(fcs), std::end(fcs), [&](auto f){ _pvis->updatePath(f, pathId);});

    fm->unlock();

    _hhover = nullptr;
    _hdrag = nullptr;
    _pvis->setCaptionsVisible( false);
    return true;
}   // end deletePath


bool PathSetInteractor::setDrag( int pid, const QPoint& p)
{
    FaceControl *fc = hoverModel();
    if ( !fc)
        return false;

    _hdrag = _pvis->pathHandle1( fc, pid);
    assert(_hdrag);
    viewer()->setCursor(Qt::CrossCursor);
    _origPos = _hdrag->pos();
    leftDrag( p);
    return true;
}   // end setDrag


bool PathSetInteractor::leftButtonDown( const QPoint& p)
{
    leftButtonUp(p);
    _hdrag = getPathViewHandle(p);  // Look for the path handle by prop under the point.
    if ( _hdrag)
    {
        viewer()->setCursor(Qt::CrossCursor);
        _origPos = _hdrag->pos();
    }   // end if
    return mouseMove(p);
}   // end leftButtonDown


bool PathSetInteractor::leftButtonUp( const QPoint& p)
{
    if ( _hdrag && _hdrag->pos() != _origPos) // Did the postion of the drag handle change?
    {
        FaceControl *fc = hoverModel();
        assert(fc);
        emit onChangedData(fc);
    }   // end if
    viewer()->setCursor(Qt::ArrowCursor);
    _hdrag = nullptr;
    return false;
}   // end leftButtonUp


bool PathSetInteractor::leftDrag( const QPoint& p)
{
    if ( !_hdrag)
        return false;

    // Get the position on the surface of the actor
    FaceControl *fc = hoverModel();
    assert(fc);
    cv::Vec3f hpos;
    if ( !viewer()->calcSurfacePosition( fc->view()->surfaceActor(), p, hpos))
        return false;

    // Update the position of the path handle
    FaceModel* fm = fc->data();
    fm->lockForWrite();
    Path* path = fm->paths()->path( _hdrag->host()->path().id);
    assert(path);
    // Which handle to move?
    if ( _hdrag == _hdrag->host()->handle0())
        *path->vtxs.begin() = hpos;
    else
        *path->vtxs.rbegin() = hpos;
    path->recalculate( fm->kdtree());

    // Update visualisation over all associated FaceControls
    for ( FaceControl* f : fm->faceControls())
    {
        _pvis->updatePath( f, path->id);
        _pvis->setCaptions( f, path->id);
    }   // end for

    fm->unlock();
    fm->updateRenderers();
    return true;
}   // end leftDrag


bool PathSetInteractor::mouseMove( const QPoint& p)
{
    if ( leftDrag(p))
        return false;

    const PathView::Handle* oldHover = _hhover;
    _hhover = _hdrag;
    if ( !_hhover)
        _hhover = getPathViewHandle(p);

    if ( oldHover != _hhover)
    {
        FaceControl *fc = hoverModel();
        int id = -1;
        if ( _hhover)
        {
            id = _hhover->host()->path().id;
            if ( fc)
            {
                cv::Vec3f hpos;
                viewer()->calcSurfacePosition( fc->view()->surfaceActor(), p, hpos);
                _pvis->setCaptions( fc, id);
            }   // end if
        }   // end if
        _pvis->setCaptionsVisible( id >= 0);

        if ( fc)
            fc->data()->updateRenderers();
    }   // end if
    return _hdrag != nullptr;
}   // end mouseMove


// private
const PathView::Handle* PathSetInteractor::getPathViewHandle( const QPoint& p) const
{
    FaceControl *fc = hoverModel();
    if ( !fc)
        return nullptr;
    return _pvis->pathHandle( fc, viewer()->getPointedAt(p));
}   // end getPathViewHandle
