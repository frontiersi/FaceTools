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

#include <Action/ActionSynchroniseCameraMovement.h>
#include <Interactor/ViewerNotifier.h>
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSynchroniseCameraMovement;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::ModelViewer;
using FaceTools::Interactor::ViewerNotifier;
using MS = FaceTools::Action::ModelSelector;


namespace {
class CameraMoveHandler : public ViewerNotifier
{
protected:
    void cameraStart() override
    {
        _ivwr = mouseViewer();
        _icam = _ivwr->camera();
    }   // end cameraStart

    // NB Only rotation and panning are synchronised using these calculations!
    void cameraMove() override
    {
        RFeatures::CameraParams cnow = _ivwr->camera();
        // Find the difference between the camera position and focus at the start of the
        // movement and those parameters now, and apply the difference to the cameras of
        // the other viewers.
        const cv::Vec3f pfNow = cnow.pos - cnow.focus;
        const cv::Vec3f pfOld = _icam.pos - _icam.focus;
        const cv::Vec3f raxis = pfNow.cross(pfOld);  // Axis of rotation
        const double n = double(pfNow.dot(pfOld));
        const double d = cv::norm(pfNow)*cv::norm(pfOld);
        const double v = std::min<double>( std::max<double>(-1, n/d), 1);
        const double rads = -acos( v);

        const cv::Vec3f tvec = cnow.focus - _icam.focus;
        RFeatures::Transformer trans( rads, raxis);

        for ( ModelViewer* v : MS::viewers())
        {
            if ( v != _ivwr)
            {
                RFeatures::CameraParams vcam = v->camera();
                cv::Vec3f pos = vcam.pos;
                const cv::Vec3f nfoc = vcam.focus + tvec;
                trans.transform(pos);
                v->setCamera( nfoc, pos);
                v->updateRender();
            }   // end if
        }   // end for
        _icam = cnow;
    }   // end cameraMove

private:
    FMV* _ivwr;
    RFeatures::CameraParams _icam;
};  // end class
}   // end namespace


// public
ActionSynchroniseCameraMovement::ActionSynchroniseCameraMovement( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _camMover(new CameraMoveHandler)
{
    setCheckable(true, false);
    _camMover->setEnabled(false);
}   // end ctor


ActionSynchroniseCameraMovement::~ActionSynchroniseCameraMovement()
{
    delete _camMover;
}   // end dtor


void ActionSynchroniseCameraMovement::doAction( Event)
{
    _camMover->setEnabled( isChecked());
}   // end doAction
