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
#include <ActionSynchroniseCameraMovement.h>
#include <FaceShapeLandmarks2DDetector.h>   // namespace FaceTools::Landmarks
#include <FaceModelViewer.h>
#include <ChangeEvents.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <Transformer.h>    // RFeatures
#include <algorithm>
using FaceTools::Action::ActionOrientCameraToFace;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::ModelViewer;


ActionOrientCameraToFace::ActionOrientCameraToFace( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _distance(450.0f), _urads(0.0f)
{
}   // end ctor


bool ActionOrientCameraToFace::testReady( const FaceControl* fc)
{
    fc->data()->lockForRead();
    const bool glmk = FaceTools::hasReqLandmarks( fc->data()->landmarks());
    fc->data()->unlock();
    return glmk;
}   // end testReady


bool ActionOrientCameraToFace::doAction( FaceControlSet& fset, const QPoint&)
{
    assert(fset.size() == 1);
    const FaceControl* fc = fset.first();
    const FaceModel* fm = fc->data();

    fm->lockForRead();
    RFeatures::Orientation on = fm->orientation();
    FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
    using namespace FaceTools::Landmarks;
    const cv::Vec3f& leye = lmks->pos( L_EYE_CENTRE);
    const cv::Vec3f& reye = lmks->pos( R_EYE_CENTRE);
    const cv::Vec3f& ntip = lmks->pos( NASAL_TIP);
    fm->unlock();

    const cv::Vec3f& uvec = on.up();
    const cv::Vec3f& nvec = on.norm();

    // Rotate the orientation about its up vector by the set amount.
    RFeatures::Transformer transformer( _urads, on.up());
    on.rotate( transformer.matrix());

    const cv::Vec3f focus = FaceTools::calcFaceCentre( leye, reye, ntip);
    fc->viewer()->setCamera( focus, nvec, uvec, _distance);

    // If camera synchroniser is null, work on just the selected FaceControl's viewer,
    // otherwise work over all viewers registered with the camera synchroniser.
    FaceTools::Action::ActionSynchroniseCameraMovement::sync();

    return true;
}   // end doAction
