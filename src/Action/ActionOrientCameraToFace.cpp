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


ActionOrientCameraToFace::ActionOrientCameraToFace( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico, true)
{
    addRespondTo( DATA_CHANGE);
    addChangeTo( VIEW_CHANGE);
}   // end ctor


bool ActionOrientCameraToFace::testReady( FaceControl* fc) { return FaceTools::hasReqLandmarks( fc->data()->landmarks());}


bool ActionOrientCameraToFace::doAction( FaceControlSet& fset)
{
    using namespace FaceTools::Landmarks;
    const FaceModelSet& fms = fset.models();
    for ( const FaceModel* fm : fms)
    {
        const cv::Vec3f& uvec = fm->orientation().up();
        const cv::Vec3f& nvec = fm->orientation().norm();
        const cv::Vec3f& leye = fm->landmarks().pos( L_EYE_CENTRE);
        const cv::Vec3f& reye = fm->landmarks().pos( R_EYE_CENTRE);
        const cv::Vec3f& ntip = fm->landmarks().pos( NASAL_TIP);
        const cv::Vec3f focus = FaceTools::calcFaceCentre( uvec, leye, reye, ntip);
        fset(fm).first()->viewer()->setCamera( focus, nvec, uvec, 480.0f);
    }   // end for
    return true;
}   // end doAction
