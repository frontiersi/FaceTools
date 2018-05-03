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

#include <ActionOrientCameraToFace.h>
#include <FaceShapeLandmarks2DDetector.h>   // namespace FaceTools::Landmarks
#include <FaceModelViewer.h>
#include <ChangeEvents.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
using FaceTools::Action::ActionOrientCameraToFace;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


ActionOrientCameraToFace::ActionOrientCameraToFace()
    : FaceAction(true), _icon( ":/icons/ORIENT_CAMERA")
{
    addRespondTo(LANDMARK_ADDED);
    addRespondTo(LANDMARK_DELETED);
    addRespondTo(LANDMARK_CHANGED);
}   // end ctor


bool ActionOrientCameraToFace::testReady( FaceControl* fc) { return FaceTools::hasReqLandmarks( fc->data()->landmarks());}


bool ActionOrientCameraToFace::doAction( FaceControlSet& fset)
{
    using namespace FaceTools::Landmarks;
    for ( const FaceControl* fc : fset)
    {
        const FaceModel* fmodel = fc->data();
        const cv::Vec3f& uvec = fmodel->orientation().up();
        const cv::Vec3f& nvec = fmodel->orientation().norm();
        const cv::Vec3f& leye = fmodel->landmarks().pos( L_EYE_CENTRE);
        const cv::Vec3f& reye = fmodel->landmarks().pos( R_EYE_CENTRE);
        const cv::Vec3f& ntip = fmodel->landmarks().pos( NASAL_TIP);
        const cv::Vec3f focus = FaceTools::calcFaceCentre( uvec, leye, reye, ntip);
        fc->viewer()->setCamera( focus, nvec, uvec, 500.0f);
    }   // end foreach
    return true;
}   // end doAction
