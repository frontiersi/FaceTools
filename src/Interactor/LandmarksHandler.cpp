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
using FaceTools::FaceLateral;
using MS = FaceTools::Action::ModelSelector;


LandmarksHandler::LandmarksHandler( LandmarksVisualisation& vis)
    : _vis(vis), _drag(-1), _hover(-1), _lat(FACE_LATERAL_MEDIAL) {}


void LandmarksHandler::doEnterProp( FV *fv, const vtkProp *p)
{
    if ( fv == MS::selectedView())
    {
        _hover = _vis.landmarkId( fv, p, _lat);    // Sets _lat as an out parameter
        // Ignore other landmarks if dragging one already
        if ( _drag < 0 && _hover >= 0)
            _enterLandmark( _hover, _lat);
    }   // end if
}   // end doEnterProp


void LandmarksHandler::doLeaveProp( FV* fv, const vtkProp* p)
{
    if ( fv == MS::selectedView())
    {
        _hover = _vis.landmarkId( fv, p, _lat);
        if ( _drag < 0)
            _leaveLandmark( _hover, _lat);
        _hover = -1;
    }   // end if
}   // end doLeaveProp


bool LandmarksHandler::leftButtonDown()
{
    _drag = _hover;
    if ( _drag >= 0)
        emit onStartedDrag(_drag);
    return _drag >= 0;
}   // end leftButtonDown


bool LandmarksHandler::leftButtonUp()
{
    bool swallowed = false;
    if ( _drag >= 0)
    {
        emit onFinishedDrag( _drag);
        if ( _hover < 0)
            _leaveLandmark( _drag, _lat);
        _drag = -1;
        swallowed = true;
    }   // end if
    return swallowed;
}   // end doOnLeftButtonUp


bool LandmarksHandler::leftDrag()
{
    bool swallowed = false;
    if ( MS::cursorView() && _drag >= 0)
    {
        // Get the position on the surface of the actor
        if ( MS::cursorView()->projectToSurface( MS::mousePos(), _dpos))
        {
            swallowed = true;
            _landmarkMove( _drag, _lat, _dpos);
        }   // end if
    }   // end if
    return swallowed;
}   // end leftDrag


void LandmarksHandler::_landmarkMove( int id, FaceLateral lat, const cv::Vec3f& pos)
{
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    fm->setLandmarkPosition( id, lat, pos); // Also updates the model's orientation
    _vis.updateLandmark( fm, id);
    MS::showStatus( posString( Landmark::LandmarksManager::makeLandmarkString( id, lat) + ": ", pos), 5000);
    MS::syncBoundingVisualisation( fm); // Reflect the orientation update in the
    fm->unlock();
    MS::setCursor(Qt::CursorShape::CrossCursor);
    MS::updateRender();
}   // end _landmarkMove


void LandmarksHandler::_enterLandmark( int id, FaceLateral lat)
{
    FM* fm = MS::selectedModel();
    _vis.setLandmarkHighlighted( fm, id, lat, true);
    const cv::Vec3f pos = fm->currentAssessment()->landmarks().pos( id, lat);
    MS::showStatus( posString( Landmark::LandmarksManager::makeLandmarkString( id, lat) + ": ", pos), 5000);
    MS::setCursor(Qt::CursorShape::CrossCursor);
    MS::updateRender();
}   // end _enterLandmark


void LandmarksHandler::_leaveLandmark( int id, FaceLateral lat)
{
    FM* fm = MS::selectedModel();
    _vis.setLandmarkHighlighted( fm, id, lat, false);
    MS::restoreCursor();
    MS::updateRender();
}   // end _leaveLandmark
