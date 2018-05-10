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

#include <ActionTransformToStandardPosition.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <ObjModelMover.h>  // RFeatures
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceView.h>
#include <VtkTools.h>
using FaceTools::Action::ActionTransformToStandardPosition;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionTransformToStandardPosition::ActionTransformToStandardPosition()
    : FaceAction(true/*disable before other*/), _icon( ":/icons/TRANSFORM")
{
    addChangeTo( MODEL_TRANSFORMED);
    addChangeTo( MODEL_ORIENTATION_CHANGED);

    // Default (parent) implementation of respondToChange adequate for these events.
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);
}   // end ctor


bool ActionTransformToStandardPosition::testReady( FaceControl* fc)
{
    return FaceTools::hasReqLandmarks( fc->data()->landmarks());
}   // end testReady


bool ActionTransformToStandardPosition::doAction( FaceControlSet& rset)
{
    using namespace FaceTools::Landmarks;
    const FaceModelSet& fms = rset.models();
    for ( FaceModel* fm : fms)
    {
        const RFeatures::Orientation& on = fm->orientation();
        const FaceTools::LandmarkSet& lmks = fm->landmarks();
        const cv::Vec3f c = FaceTools::calcFaceCentre( on.up(), lmks.pos(L_EYE_CENTRE), lmks.pos(R_EYE_CENTRE), lmks.pos(NASAL_TIP));
        cv::Matx44d m = RFeatures::toStandardPosition( on, c);
        fm->transform(m);   // Transform the data - will cause all associated views to update
    }   // end for
    return true;
}   // end doAction
