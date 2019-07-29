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

#include <LandmarksInteractor.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceView.h>
using FaceTools::Interactor::FaceViewInteractor;
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::FaceLateral;
using MS = FaceTools::Action::ModelSelector;


// public
LandmarksInteractor::LandmarksInteractor( LandmarksVisualisation& vis)
    : _vis(vis), _drag(-1), _hover(-1), _lat(FACE_LATERAL_MEDIAL) { }


void LandmarksInteractor::enterProp( FV *fv, const vtkProp *p)
{
    if ( fv == MS::selectedView())
    {
        _hover = _vis.landmarkId( fv, p, _lat);    // Sets _lat as an out parameter
        // Ignore other landmarks if dragging one already
        if ( _drag < 0 && _hover >= 0)
            enterLandmark( _hover, _lat);
    }   // end if
}   // end enterProp


void LandmarksInteractor::leaveProp( FV* fv, const vtkProp* p)
{
    if ( fv == MS::selectedView())
    {
        _hover = _vis.landmarkId( fv, p, _lat);
        if ( _drag < 0)
            leaveLandmark( _hover, _lat);
        _hover = -1;
    }   // end if
}   // end leaveProp


bool LandmarksInteractor::leftButtonDown()
{
    _drag = _hover;
    if ( _drag >= 0)
        emit onStartedDrag(_drag);
    return _drag >= 0;
}   // end leftButtonDown


bool LandmarksInteractor::leftButtonUp()
{
    bool swallowed = false;
    if ( _drag >= 0)
    {
        emit onFinishedDrag( _drag);
        if ( _hover < 0)
            leaveLandmark( _drag, _lat);
        _drag = -1;
        swallowed = true;
    }   // end if
    return swallowed;
}   // end doOnLeftButtonUp


bool LandmarksInteractor::leftDrag()
{
    bool swallowed = false;
    if ( view() && _drag >= 0)
    {
        // Get the position on the surface of the actor
        if ( view()->projectToSurface( MS::mousePos(), _dpos))
        {
            swallowed = true;
            landmarkMove( _drag, _lat, _dpos);
        }   // end if
    }   // end if
    return swallowed || FaceViewInteractor::leftDrag();
}   // end leftDrag


void LandmarksInteractor::landmarkMove( int id, FaceLateral lat, const cv::Vec3f& pos)
{
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    fm->setLandmarkPosition( id, lat, pos);
    _vis.updateLandmark( fm, id);
    MS::syncBoundingVisualisation( fm);
    fm->unlock();
    MS::setCursor(Qt::CursorShape::CrossCursor);
    MS::updateRender();
}   // end landmarkMove


void LandmarksInteractor::enterLandmark( int id, FaceLateral lat)
{
    FM* fm = MS::selectedModel();
    _vis.setLandmarkHighlighted( fm, id, lat, true);
    MS::setCursor(Qt::CursorShape::CrossCursor);
}   // end enterLandmark


void LandmarksInteractor::leaveLandmark( int id, FaceLateral lat)
{
    FM* fm = MS::selectedModel();
    _vis.setLandmarkHighlighted( fm, id, lat, false);
    MS::restoreCursor();
}   // end leaveLandmark
