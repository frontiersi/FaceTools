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

#include <Interactor/PropNotifier.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::PropNotifier;
using FaceTools::Vis::FV;

PropNotifier::PropNotifier() : _mnow(nullptr), _pnow(nullptr) {}

bool PropNotifier::mouseMove() { testMouse(); return false;}
bool PropNotifier::leftDrag() { testMouse(); return false;}
bool PropNotifier::middleDrag() { testMouse(); return false;}
bool PropNotifier::rightDrag() { testMouse(); return false;}

void PropNotifier::testMouse()
{
    const FMV *fmv = mouseViewer();
    const QPoint p = fmv->mouseCoords();
    const vtkProp* pnow = p.x() < 0 ? nullptr : fmv->getPointedAt(p);   // The prop pointed at (may not be on current model)

    // The FaceView that the prop belongs to (if any)
    FV* fv = pnow ? fmv->attached().find(pnow) : nullptr;
    if ( fv && pnow == fv->actor())
        pnow = nullptr;

    if ( _pnow && (_pnow != pnow || _mnow != fv))  // Leave previous prop?
    {
        assert( _mnow);
        emit onLeaveProp( _mnow, _pnow);
    }   // end if

    if ( _mnow != fv)
    {
        if ( _mnow)
            emit onLeaveModel( _mnow);
        _mnow = fv; // Must be updated before signal fired!
        if ( fv)    // Inform entering model
            emit onEnterModel( fv);
    }   // end if

    if ( _pnow != pnow)
    {
        _pnow = pnow;   // Must be updated before signal is fired!
        if ( pnow)      // Inform entering prop
        {
            assert(fv); // Must be true because all props must belong to a FaceView
            emit onEnterProp( fv, pnow);
        }   // end if
    }   // end if
}   // end testMouse
