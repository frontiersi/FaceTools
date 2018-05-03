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

#include <MeasureInteractor.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::MeasureInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::SurfacePathSet;
using FaceTools::SurfacePath;


// public
MeasureInteractor::MeasureInteractor( SurfacePathSet* pset)
    : ModelViewerInteractor( pset->faceControl()->viewer()),
      _paths(pset), _handle(NULL), _hhandle(NULL), _lock(false)
{
    assert(_paths);
}   // end ctor


void MeasureInteractor::leftButtonDown( const QPoint& p)
{
    _handle = NULL;
    const vtkProp* prop = _paths->faceControl()->viewer()->getPointedAt(p);
    if ( _paths->faceControl()->view()->belongs(prop) == NULL)    // Do nothing if not pointing at the face
        return;

    _lock = isInteractionLocked();
    setInteractionLocked( true);
    _handle = _paths->path(p); // Get the path clicked on (if any)/* _handle = _pdrawer->handle( p);*/
    if ( !_handle) // No path clicked on so create a new one
    {
        _handle = _paths->createPath(p);
        emit onUpdate( _handle);
    }   // end if
    mouseMove(p);
}   // end leftButtonDown


void MeasureInteractor::leftButtonUp( const QPoint&)
{
    _handle = NULL;
    setInteractionLocked( _lock);
}   // end leftButtonUp


void MeasureInteractor::leftDrag( const QPoint& p)
{
    if ( _handle != NULL)
    {
        _paths->updatePath( _handle, p);
        emit onUpdate( _handle);
    }   // end if
    else
        mouseMove(p);
}   // end leftDrag


void MeasureInteractor::mouseMove( const QPoint& p)
{
    SurfacePath::Handle* oldHover = _hhandle;
    _hhandle = _paths->path(p);
    if ( oldHover != _hhandle)
        emit onHover( _hhandle);
}   // end mouseMove


void MeasureInteractor::rightButtonDown( const QPoint& p)
{
    SurfacePath::Handle* handle = _paths->path(p);
    if ( handle)
        emit onRightButtonDown( handle);
}   // end rightButtonDown

            //_cmenu->exec( viewer()->mapToGlobal(p));
