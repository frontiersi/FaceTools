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

#include <Interactor/LandmarksHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <Action/ModelSelector.h>
#include <Vis/FaceView.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Vec3f;
using FaceTools::FaceLateral;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


LandmarksHandler::Ptr LandmarksHandler::create() { return Ptr( new LandmarksHandler);}

// private
LandmarksHandler::LandmarksHandler()
    : _hoverId(-1), _dragId(-1), _lat(FACE_LATERAL_MEDIAL) {}


void LandmarksHandler::refreshState()
{
    setEnabled( MS::isViewSelected() && MS::selectedView()->isApplied(&_vis));
}   // end refreshState


void LandmarksHandler::doEnterProp( FV *fv, const vtkProp *p)
{
    if ( fv == MS::selectedView())
    {
        const int hid = _vis.landmarkId( fv, p, _lat);    // Sets _lat as an out parameter
        if ( _dragId < 0 && hid >= 0) // Ignore other landmarks if dragging one already
        {
            _hoverId = hid;
            _vis.setLabelVisible( fv, hid, _lat, true);
            _vis.setLandmarkHighlighted( fv, hid, _lat, true);
            const Vec3f& pos = fv->data()->currentLandmarks().pos( hid, _lat);
            MS::showStatus( posString( LMAN::makeLandmarkString( hid, _lat), pos), 5000);
            MS::setCursor( Qt::CursorShape::CrossCursor);
            emit onEnterLandmark( hid, _lat);
        }   // end if
    }   // end if
}   // end doEnterProp


void LandmarksHandler::doLeaveProp( FV* fv, const vtkProp* p)
{
    if ( _dragId < 0)
    {
        FaceLateral lat;
        const int hid = _vis.landmarkId( fv, p, lat);
        if ( lat == _lat && hid == _hoverId)
            _leaveLandmark();
    }   // end if
}   // end doLeaveProp


void LandmarksHandler::_leaveLandmark()
{
    const FV *fv = MS::selectedView();
    _vis.setLabelVisible( fv, _hoverId, _lat, false);
    _vis.setLandmarkHighlighted( fv, _hoverId, _lat, false);
    MS::restoreCursor();
    emit onLeaveLandmark( _hoverId, _lat); 
    _hoverId = -1;
    _lat = FACE_LATERAL_MEDIAL;
}   // end _leaveLandmark


bool LandmarksHandler::leftButtonDown()
{
    FaceLateral lat;
    const int lmid = _vis.landmarkId( MS::selectedView(), MS::cursorProp(), lat);
    if ( lmid >= 0 && lmid == _hoverId && lat == _lat && !LMAN::landmark( lmid)->isLocked())
    {
        _dragId = lmid;
        emit onStartedDrag( _dragId, _lat);
    }   // end if
    return _dragId >= 0;
}   // end leftButtonDown


bool LandmarksHandler::leftButtonUp()
{
    bool swallowed = false;
    if ( _dragId >= 0)
    {
        emit onFinishedDrag( _dragId, _lat);
        _dragId = -1;
        swallowed = true;
    }   // end if
    return swallowed;
}   // end leftButtonUp


bool LandmarksHandler::leftDrag()
{
    bool swallowed = false;
    if ( _dragId >= 0)
    {
        // Get the position on the surface of the actor
        const FV *fv = MS::selectedView();
        Vec3f dpos;
        if ( fv->projectToSurface( fv->viewer()->mouseCoords(), dpos))
        {
            FM* fm = fv->data();
            fm->lockForWrite();
            fm->setLandmarkPosition( _dragId, _lat, dpos);
            for ( const auto& f : fm->fvs())
                _vis.refreshLandmark( f, _dragId);
            fm->unlock();

            MS::showStatus( posString( LMAN::makeLandmarkString( _dragId, _lat), dpos), 5000);
            MS::setCursor( Qt::CursorShape::CrossCursor);
            emit onDoingDrag( _dragId, _lat);
            swallowed = true;
        }   // end if
    }   // end if
    return swallowed;
}   // end leftDrag
