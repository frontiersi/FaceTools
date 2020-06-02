/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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
using FaceTools::Vec3f;


namespace {
class CameraMoveHandler : public ViewerNotifier
{
public:
    void update()
    {
        cameraStart();
        cameraMove();
    }   // end update

protected:
    void cameraStart() override
    {
        _ivwr = MS::selectedViewer();
        _icam = _ivwr->camera();
    }   // end cameraStart

    // NB Only rotation and panning are synchronised using these calculations!
    void cameraMove() override
    {
        r3d::CameraParams cnow = _ivwr->camera();
        /*
        // Find the difference between the camera position and focus at the start of the
        // movement and those parameters now, and apply the difference to the cameras of
        // the other viewers.
        const Vec3f pfNow = cnow.pos() - cnow.focus();
        const Vec3f pfOld = _icam.pos() - _icam.focus();
        const Vec3f raxis = pfNow.cross(pfOld);  // Axis of rotation
        const float n = pfNow.dot(pfOld);
        const float d = pfNow.norm() * pfOld.norm();
        const float v = std::min<float>( std::max<float>(-1, n/d), 1);
        const float rads = -acosf( v);

        const Vec3f tvec = cnow.focus() - _icam.focus();
        r3d::Transformer trans( rads, raxis);
        */

        for ( ModelViewer* v : MS::viewers())
        {
            if ( v != _ivwr)
            {
                v->setCamera(cnow);
                /*
                r3d::CameraParams vcam = v->camera();
                Vec3f pos = vcam.pos();
                const Vec3f nfoc = vcam.focus() + tvec;
                trans.transform(pos);
                v->setCamera( nfoc, pos);
                */
                v->updateRender();
            }   // end if
        }   // end for
        _icam = cnow;
    }   // end cameraMove

private:
    FMV* _ivwr;
    r3d::CameraParams _icam;
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


bool ActionSynchroniseCameraMovement::isAllowed(Event)
{
    return MS::isViewSelected();
}   // end isAllowed


bool ActionSynchroniseCameraMovement::checkState( Event e)
{
    const bool isOn = _camMover->isEnabled();
    if ( isOn && has( e, Event::CAMERA_CHANGE))
        static_cast<CameraMoveHandler*>(_camMover)->update();
    return isOn;
}   // end checkState


void ActionSynchroniseCameraMovement::doAction( Event)
{
    _camMover->setEnabled( isChecked());
    if ( isChecked())
        static_cast<CameraMoveHandler*>(_camMover)->update();
}   // end doAction
