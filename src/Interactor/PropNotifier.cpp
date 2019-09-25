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

#include <Interactor/PropNotifier.h>
#include <Action/ModelSelector.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::PropNotifier;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using MS = FaceTools::Action::ModelSelector;

PropNotifier::PropNotifier() : _mnow(nullptr), _pnow(nullptr) {}

bool PropNotifier::mouseMove() { return _testPoint();}
bool PropNotifier::leftDrag() { return _testPoint();}
bool PropNotifier::middleDrag() { return _testPoint();}
bool PropNotifier::rightDrag() { return _testPoint();}

bool PropNotifier::_testPoint()
{
    const QPoint p = MS::mousePos();
    const vtkProp* pnow = p.x() < 0 ? nullptr : mouseViewer()->getPointedAt(p);   // The prop pointed at (may not be on current model)

    // The FaceView that the prop belongs to (if any)
    FV* fv = pnow ? static_cast<const FMV*>( mouseViewer())->attached().find(pnow) : nullptr;
    if ( fv && pnow == fv->actor())
        pnow = nullptr;

    if ( _pnow && (_pnow != pnow || _mnow != fv))  // Leave previous prop?
    {
        assert( _mnow);
        //std::cerr << "LEAVING prop " << std::hex << _pnow << " on face " << _mnow << std::endl;
        emit onLeaveProp( _mnow, _pnow);
    }   // end if

    if ( _mnow != fv)
    {
        if ( _mnow)
        {
            //std::cerr << "Leaving model" << std::endl;
            emit onLeaveModel( _mnow);
        }   // end if
        _mnow = fv; // Must be updated before signal fired!
        if ( fv)    // Inform entering model
        {
            //std::cerr << "Entered model" << std::endl;
            emit onEnterModel( fv);
        }   // end if
    }   // end if

    if ( _pnow != pnow)
    {
        _pnow = pnow;   // Must be updated before signal is fired!
        if ( pnow)      // Inform entering prop
        {
            assert(fv); // Must be true because all props must belong to a FaceView
            //std::cerr << "Interactor (" << std::hex << this << " enterProp " << pnow << std::endl;
            emit onEnterProp( fv, pnow);
        }   // end if
    }   // end if

    return false;
}   // end _testPoint
