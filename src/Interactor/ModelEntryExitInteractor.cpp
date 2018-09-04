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

#include <ModelEntryExitInteractor.h>
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::ModelEntryExitInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Vis::FV;
using FaceTools::FMV;


// public
ModelEntryExitInteractor::ModelEntryExitInteractor()
    : _mnow(nullptr), _pnow(nullptr), _ldown(false)
{
}   // end ctor


// protected
void ModelEntryExitInteractor::onAttached()
{
    _viewer = qobject_cast<FaceModelViewer*>( viewer());
    assert(_viewer);    // Check that this was added to a FaceModelViewer and not a base ModelViewer
    if ( !_viewer)
    {
        std::cerr << "[ERROR] FaceTools::Interactor::ModelEntryExitInteractor::onAttached: "
                  << "Must be attached to FaceModelViewer!" << std::endl;
    }   // end if
}   // end onAttached


// protected
void ModelEntryExitInteractor::onDetached()
{
    _viewer = nullptr;
    testLeaveProp(nullptr, nullptr);
    testLeaveModel();
    _pnow = nullptr;
}   // end onDetached


// private virtual
bool ModelEntryExitInteractor::leftDrag( const QPoint& p) { emit onLeftDrag(); return testPoint(p);}
bool ModelEntryExitInteractor::rightDrag( const QPoint& p) { return testPoint(p);}
bool ModelEntryExitInteractor::middleDrag( const QPoint& p) { return testPoint(p);}
bool ModelEntryExitInteractor::mouseMove( const QPoint& p) { return testPoint(p);}
bool ModelEntryExitInteractor::mouseLeave( const QPoint& p) { return testPoint(p);}
bool ModelEntryExitInteractor::mouseEnter( const QPoint& p) { return testPoint(p);}


// private virtual
bool ModelEntryExitInteractor::rightButtonDown( const QPoint& p) { return testPoint(p);}


// private virtual
bool ModelEntryExitInteractor::leftButtonDown( const QPoint& p)
{
    _ldown = true;
    emit onLeftDown();
    testPoint(p);
    return false;
}   // end leftButtonDown


// private virtual
bool ModelEntryExitInteractor::leftButtonUp( const QPoint&)
{
    _ldown = false;
    emit onLeftUp();
    return false;
}   // end leftButtonUp


// private
bool ModelEntryExitInteractor::testPoint( const QPoint& p)
{
    const vtkProp* pnow = _viewer->getPointedAt(p);     // The prop pointed at (may not be on current model)
    FV* fv = _viewer->attached().find(pnow);            // The FaceControl that the prop belongs to (if any)

    testLeaveProp( fv, pnow);

    if ( _mnow != fv)
    {
        testLeaveModel();
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
            //std::cerr << "Entered prop (model is " << fv << ") " << "viewer has " << _viewer->attached().size() << " FVs" << std::endl;
            assert(fv);
            emit onEnterProp( fv, pnow);
        }   // end if
    }   // end if

    return false;
}   // end testPoint


void ModelEntryExitInteractor::testLeaveProp( FV* fv, const vtkProp* pnow)
{
    if ( _pnow && (_pnow != pnow || _mnow != fv))  // Leave previous prop?
    {
        //std::cerr << "Leaving prop (model is " << _mnow << ")" << std::endl;
        assert( _mnow);
        emit onLeaveProp( _mnow, _pnow);
    }   // end if
}   // end testLeaveProp


void ModelEntryExitInteractor::testLeaveModel()
{
    if ( _mnow) // Inform leaving model
    {
        //std::cerr << "Leaving model " << _mnow << std::endl;
        emit onLeaveModel( _mnow);
        _mnow = nullptr;
    }   // end if
}   // end testLeaveModel
