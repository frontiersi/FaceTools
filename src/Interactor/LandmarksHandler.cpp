/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <ModelSelect.h>
#include <Vis/FaceView.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::FV;
using FaceTools::Vec3f;
using FaceTools::FaceSide;
using MS = FaceTools::ModelSelect;
using LMAN = FaceTools::Landmark::LandmarksManager;


LandmarksHandler::Ptr LandmarksHandler::create() { return Ptr( new LandmarksHandler);}

// private
LandmarksHandler::LandmarksHandler() : _hoverId(-1), _dragId(-1), _lat(MID), _emitDragUpdates(false) {}


void LandmarksHandler::refresh()
{
    const FV *fv = MS::selectedView();
    setEnabled( fv && _vis.isVisible(fv));
    if ( !isEnabled())
        _leaveLandmark();
}   // end refresh


bool LandmarksHandler::doEnterProp()
{
    const FV *fv = MS::selectedView();
    const vtkProp *p = this->prop();
    FaceSide lat;
    const int hid = _vis.landmarkId( fv, p, lat);    // Sets _lat as an out parameter
    bool swallowed = false;
    if ( _dragId < 0 && hid >= 0) // Ignore other landmarks if dragging one already
    {
        swallowed = true;
        _hoverId = hid;
        _lat = lat;
        _vis.setLabelVisible( fv, hid, _lat, true);
        _vis.setLandmarkHighlighted( fv, hid, _lat, true);
        const Vec3f pos = fv->rdata()->currentLandmarks().pos( hid, _lat);
        MS::showStatus( posString( LMAN::makeLandmarkString( hid, _lat), pos), 5000);
        MS::setCursor( Qt::CursorShape::CrossCursor);
        emit onEnterLandmark( hid, _lat);
    }   // end if
    return swallowed;
}   // end doEnterProp


bool LandmarksHandler::doLeaveProp()
{
    bool swallowed = false;
    const FV *fv = MS::selectedView();
    const vtkProp *p = this->prop();
    FaceSide lat;
    const int hid = _vis.landmarkId( fv, p, lat);
    if ( _dragId < 0 && hid >= 0)
    {
        swallowed = true;
        _leaveLandmark();
    }   // end if
    return swallowed;
}   // end doLeaveProp


void LandmarksHandler::_leaveLandmark()
{
    const FV *fv = MS::selectedView();
    if ( fv && _hoverId >= 0)
    {
        MS::restoreCursor();
        _vis.setLabelVisible( fv, _hoverId, _lat, false);
        _vis.setLandmarkHighlighted( fv, _hoverId, _lat, false);
        emit onLeaveLandmark( _hoverId, _lat); 
    }   // end if
    _dragId = _hoverId = -1;
    _lat = MID;
}   // end _leaveLandmark


bool LandmarksHandler::doLeftButtonDown()
{
    bool swallowed = false;
    FaceSide lat;
    const int lmid = _vis.landmarkId( MS::selectedView(), this->prop(), lat);
    if ( lmid >= 0 && lmid == _hoverId && lat == _lat && !LMAN::landmark( lmid)->isLocked())
    {
        swallowed = true;
        _dragId = lmid;
        emit onStartedDrag( _dragId, _lat);
    }   // end if
    return swallowed;
}   // end doLeftButtonDown


bool LandmarksHandler::doLeftButtonUp()
{
    bool swallowed = false;
    if ( _dragId >= 0)
    {
        swallowed = true;
        emit onFinishedDrag( _dragId, _lat);
        // Deal with the case where mouse button is released with cursor off the landmark
        // (because landmark was restricted in movement).
        FaceSide lat;
        const int lmid = _vis.landmarkId( MS::selectedView(), this->prop(), lat);
        if ( lmid < 0 && _hoverId >= 0)
            _leaveLandmark();
        _dragId = -1;
    }   // end if
    return swallowed;
}   // end doLeftButtonUp


bool LandmarksHandler::doLeftDrag()
{
    bool swallowed = false;
    if ( _dragId >= 0)
    {
        swallowed = true;
        // Get the position on the surface of the actor
        FV *fv = MS::selectedView();
        Vec3f dpos;
        if ( fv->projectToSurface( fv->viewer()->mouseCoords(), dpos))
        {
            FM *fm = fv->data();
            fm->lockForWrite();
            fm->setLandmarkPosition( _dragId, _lat, dpos);
            for ( const FV *f : fm->fvs())
                _vis.refreshLandmarkPosition( f, _dragId, _lat);
            fm->unlock();
            // Update across all viewers if more than one view
            MS::showStatus( posString( LMAN::makeLandmarkString( _dragId, _lat), dpos), 5000);
            MS::setCursor( Qt::CursorShape::CrossCursor);
            if ( _emitDragUpdates)
                emit onDoingDrag( _dragId, _lat);
            else if ( fm->fvs().size() > 1)
                MS::updateRender();
        }   // end if
    }   // end if
    return swallowed;
}   // end doLeftDrag
