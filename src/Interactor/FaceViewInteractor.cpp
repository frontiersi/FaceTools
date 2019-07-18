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

#include <FaceViewInteractor.h>
#include <FaceModelViewer.h>
#include <ModelSelector.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::FaceViewInteractor;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using MS = FaceTools::Action::ModelSelector;


FaceViewInteractor::FaceViewInteractor() : _mnow(nullptr), _pnow(nullptr) {}

void FaceViewInteractor::enterViewer( FMV*) { testPoint();}
void FaceViewInteractor::leaveViewer( FMV*) { testPoint();}
bool FaceViewInteractor::mouseMove() { return testPoint();}
bool FaceViewInteractor::leftButtonDown() { mouseViewer()->updateRender(); return false;}
bool FaceViewInteractor::leftDoubleClick() { mouseViewer()->updateRender(); return false;}
bool FaceViewInteractor::leftDrag() { return testPoint();}
bool FaceViewInteractor::middleDrag() { return testPoint();}
bool FaceViewInteractor::rightDrag() { return testPoint();}

bool FaceViewInteractor::testPoint()
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
        leaveProp( _mnow, _pnow);
    }   // end if

    mouseViewer()->updateRender();

    if ( _mnow != fv)
    {
        if ( _mnow)
        {
            //std::cerr << "Leaving model" << std::endl;
            leaveModel( _mnow);
        }   // end if
        _mnow = fv; // Must be updated before signal fired!
        if ( fv)    // Inform entering model
        {
            //std::cerr << "Entered model" << std::endl;
            enterModel( fv);
        }   // end if
    }   // end if

    mouseViewer()->updateRender();

    if ( _pnow != pnow)
    {
        _pnow = pnow;   // Must be updated before signal is fired!
        if ( pnow)      // Inform entering prop
        {
            assert(fv);
            enterProp( fv, pnow);
        }   // end if
    }   // end if

    mouseViewer()->updateRender();

    return false;
}   // end testPoint
