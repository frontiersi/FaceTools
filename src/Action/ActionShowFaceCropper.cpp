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

#include <ActionShowFaceCropper.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <ObjModelMover.h>  // RFeatures
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceView.h>
#include <VtkTools.h>
using FaceTools::Action::ActionShowFaceCropper;
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCrop;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionShowFaceCropper::ActionShowFaceCropper( ActionCrop* cropper)
    : FaceAction(true/*disable before other*/), _icon( cropper->getIcon()), _cropper(cropper)
{
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);
}   // end ctor


bool ActionShowFaceCropper::testReady( FaceControl* fc)
{
    return FaceTools::hasReqLandmarks( fc->data()->landmarks());
}   // end testReady


bool ActionShowFaceCropper::doAction( FaceControlSet& rset)
{
    FaceModelSet tset;
    for ( FaceControl* fc : rset)
    {
        FaceModel* fm = fc->data();
        const RFeatures::Orientation& on = fm->orientation();
        const FaceTools::LandmarkSet& lmks = fm->landmarks();
        using namespace FaceTools::Landmarks;
        cv::Vec3f c = FaceTools::calcFaceCentre( on.up(), lmks.pos(L_EYE_CENTRE), lmks.pos(R_EYE_CENTRE), lmks.pos(NASAL_TIP));
        cv::Matx44d m = RFeatures::toStandardPosition( on, c);
        if ( tset.count(fm) == 0)   // Transform the model if not done already
        {
            fm->transform(m);
            tset.insert(fm);
        }   // end if
        fc->view()->transform( RVTK::toVTK(m)); // Transform the visualisations
    }   // end for
    return true;
}   // end doAction
