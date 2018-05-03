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

#include <FaceEntryExitInteractor.h>
#include <LandmarksVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::FaceEntryExitInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControl;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::LandmarksVisualisation;


// public
FaceEntryExitInteractor::FaceEntryExitInteractor( FaceModelViewer* viewer)
    : ModelViewerInteractor(viewer),
      _mnow(NULL), _lnow(-1)
{
}   // end ctor


// private virtual
void FaceEntryExitInteractor::leftDrag( const QPoint& p) {testPoint(p);}
void FaceEntryExitInteractor::rightDrag( const QPoint& p) {testPoint(p);}
void FaceEntryExitInteractor::middleDrag( const QPoint& p) {testPoint(p);}
void FaceEntryExitInteractor::mouseMove( const QPoint& p) {testPoint(p);}


// private
void FaceEntryExitInteractor::testPoint( const QPoint& p)
{
    const vtkProp* prop = _viewer->getPointedAt(p);     // The prop pointed at
    FaceControl* fc = _viewer->attached().find(prop);   // The FaceControl pointed at (if any)

    int lnow = -1;  // Will be the ID of the landmark pointed at now (may not be on current model)
    if ( fc)
    {
        const BaseVisualisation* vis = fc->view()->belongs(prop);
        if ( vis)
        {
            const LandmarksVisualisation* lvis = qobject_cast<const LandmarksVisualisation*>(vis);
            if ( lvis)  // Cast only works if prop pointed at was a landmark
                lnow = lvis->landmarks( fc)->pointedAt( p); // Get the landmark being pointed at
        }   // end if
    }   // end if

    if ( _lnow >= 0 && (_lnow != lnow || _mnow != fc))  // Leave previous landmark?
    {
        assert(_mnow);
        emit onLeaveLandmark( _mnow, _lnow);
    }   // end if

    if ( _mnow != fc)
    {
        if ( _mnow) // Inform leaving model
            emit onLeaveModel( _mnow);
        if ( fc)    // Inform entering model
            emit onEnterModel( fc);
    }   // end if

    if ( lnow >= 0 && (_lnow != lnow))  // Enter new landmark?
    {
        assert( fc);
        emit onEnterLandmark( fc, lnow);
    }   // end if

    _lnow = lnow;
    _mnow = fc;
}   // end testPoint
