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

#include <RadialSelectInteractor.h>
#include <RadialSelectVisualisation.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::FaceHoveringInteractor;
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::FaceControl;


// public
RadialSelectInteractor::RadialSelectInteractor( FEEI* feei, RadialSelectVisualisation* vis)
    : FaceHoveringInteractor( feei, vis), _vis(vis), _move(false)
{
}   // end ctor


// Double-click to reposition centre of boundary.
bool RadialSelectInteractor::leftDoubleClick( const QPoint& p)
{
    assert( hoverModel());
    _move = true;
    return leftDrag(p);
}   // end leftDoubleClick


// Left drag (but only after _move flag set with double-click)
// to reposition the centre of the boundary.
bool RadialSelectInteractor::leftDrag( const QPoint& p)
{
    assert( hoverModel());
    bool swallowed = false;
    if ( _move)
    {
        swallowed = true;
        emit onSetNewCentre( hoverModel(), viewer()->project(p));
    }   // end if
    return swallowed;
}   // end leftDrag


bool RadialSelectInteractor::leftButtonUp( const QPoint&)
{
    assert( hoverModel());
    _move = false;
    return false;
}   // end leftButtonUp


// Increase the radius
bool RadialSelectInteractor::mouseWheelForward( const QPoint& p)
{
    FaceControl* fc = hoverModel();
    assert(fc);
    emit onSetNewRadius( fc, _vis->radius(fc->data())+1);
    return true;
}   // end mouseWheelForward


// Decrease the radius
bool RadialSelectInteractor::mouseWheelBackward( const QPoint& p)
{
    FaceControl* fc = hoverModel();
    assert(fc);
    emit onSetNewRadius( fc, _vis->radius(fc->data())-1);
    return true;
}   // end mouseWheeBackward
