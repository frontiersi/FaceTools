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

#include <ModelEntryExitInteractor.h>
#include <LandmarksVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::ModelEntryExitInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControl;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::LandmarksVisualisation;


// public
ModelEntryExitInteractor::ModelEntryExitInteractor()
    : _mnow(nullptr), _lnow(-1), _ldown(false)
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
    testLeaveLandmark(nullptr,-1);
    testLeaveModel();
    _lnow = -1;
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
    const vtkProp* prop = _viewer->getPointedAt(p);     // The prop pointed at
    FaceControl* fc = _viewer->attached().find(prop);   // The FaceControl that the prop belongs to (if any)

    int lnow = -1;  // Will be the ID of the landmark pointed at now (may not be on current model)
    if ( fc)
    {
        const BaseVisualisation* vis = fc->view()->belongs(prop);
        if ( vis)
        {
            const LandmarksVisualisation* lvis = qobject_cast<const LandmarksVisualisation*>(vis);
            if ( lvis)  // Cast only works if prop pointed at was a landmark
                lnow = lvis->landmarkProp( fc, prop);
        }   // end if
    }   // end if

    testLeaveLandmark( fc, lnow);

    if ( _mnow != fc)
    {
        testLeaveModel();
        _mnow = fc; // Must be updated before signal fired!
        if ( fc)    // Inform entering model
            emit onEnterModel( fc);
    }   // end if

    if ( _lnow != lnow)
    {
        _lnow = lnow;   // Must be updated before signal is fired!
        if ( lnow >= 0) // Inform entering landmark
            emit onEnterLandmark( fc, lnow);
    }   // end if

    return false;
}   // end testPoint


void ModelEntryExitInteractor::testLeaveLandmark( FaceControl* fc, int lnow)
{
    if ( _lnow >= 0 && (_lnow != lnow || _mnow != fc))  // Leave previous landmark?
    {
        assert( _mnow);
        emit onLeaveLandmark( _mnow, _lnow);
    }   // end if
}   // end testLeaveLandmark


void ModelEntryExitInteractor::testLeaveModel()
{
    if ( _mnow) // Inform leaving model
    {
        emit onLeaveModel( _mnow);
        _mnow = nullptr;
    }   // end if
}   // end testLeaveModel
