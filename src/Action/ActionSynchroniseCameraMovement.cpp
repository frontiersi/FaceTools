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

#include <ActionSynchroniseCameraMovement.h>
#include <FaceControlSet.h>
#include <ModelViewer.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSynchroniseCameraMovement;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::ModelViewer;
using FaceTools::Interactor::ModelMoveInteractor;


const ActionSynchroniseCameraMovement* ActionSynchroniseCameraMovement::s_obj(nullptr);


// public
ActionSynchroniseCameraMovement::ActionSynchroniseCameraMovement( const QString& dn, const QIcon& ico, ModelMoveInteractor* mmi)
    : FaceAction( dn, ico), _interactor(mmi)
{
    assert( !s_obj);
    if ( s_obj)
    {
        std::cerr << "[ERROR] FaceTools::Action::ActionSynchroniseCameraMovement::ctor: More than one object of this type is bad!" << std::endl;
        delete s_obj;
    }   // end if
    setCheckable(true, false);
    s_obj = this;
}   // end ctor


bool ActionSynchroniseCameraMovement::doAction( FaceControlSet&)
{
    if ( isChecked())
    {
        connect( _interactor, &ModelMoveInteractor::onCameraRotate, this, &ActionSynchroniseCameraMovement::doOnCameraMove);
        connect( _interactor, &ModelMoveInteractor::onCameraDolly, this, &ActionSynchroniseCameraMovement::doOnCameraMove);
        connect( _interactor, &ModelMoveInteractor::onCameraPan, this, &ActionSynchroniseCameraMovement::doOnCameraMove);
    }   // end if
    else
        _interactor->disconnect(this);
    return true;
}   // end doAction


// private
void ActionSynchroniseCameraMovement::doOnCameraMove()
{
    ModelViewer* viewer = _interactor->viewer();
    RFeatures::CameraParams cp = viewer->getCamera();
    for ( ModelViewer* v : _viewers)
    {
        if ( v != viewer)   // Set the same camera parameters for v
        {
            v->setCamera(cp);
            v->updateRender();
        }   // end if
    }   // end for
}   // end doOnCameraMove
