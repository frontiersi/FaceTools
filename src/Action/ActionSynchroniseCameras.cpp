/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionSynchroniseCameras.h>
#include <Interactor/ViewerNotifier.h>
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSynchroniseCameras;
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
    }   // end cameraStart

    void cameraMove() override
    {
        r3d::CameraParams cnow = _ivwr->camera();
        const double pscale = _ivwr->getRenderer()->GetActiveCamera()->GetParallelScale();

        for ( ModelViewer* v : MS::viewers())
        {
            if ( v != _ivwr)
            {
                v->setCamera(cnow);
                v->getRenderer()->GetActiveCamera()->SetParallelScale( pscale);
                v->updateRender();
            }   // end if
        }   // end for
    }   // end cameraMove

private:
    FMV* _ivwr;
};  // end class
}   // end namespace


// public
ActionSynchroniseCameras::ActionSynchroniseCameras( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _camMover(new CameraMoveHandler)
{
    setCheckable(true, false);
    _camMover->setEnabled(false);
}   // end ctor


ActionSynchroniseCameras::~ActionSynchroniseCameras()
{
    delete _camMover;
    _camMover = nullptr;
}   // end dtor


bool ActionSynchroniseCameras::isAllowed(Event)
{
    return MS::isViewSelected();
}   // end isAllowed


bool ActionSynchroniseCameras::checkState( Event e)
{
    const bool isOn = _camMover->isEnabled() && MS::isViewSelected();
    if ( isOn && has( e, Event::CAMERA_CHANGE))
        static_cast<CameraMoveHandler*>(_camMover)->update();
    return isOn;
}   // end checkState


void ActionSynchroniseCameras::doAction( Event)
{
    _camMover->setEnabled( isChecked());
    if ( isChecked())
        static_cast<CameraMoveHandler*>(_camMover)->update();
}   // end doAction
