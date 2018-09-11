/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSynchroniseCameraMovement;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::ModelViewer;
using FaceTools::Interactor::ModelMoveInteractor;


// public
ActionSynchroniseCameraMovement::ActionSynchroniseCameraMovement( const QString& dn, const QIcon& ico, ModelMoveInteractor* mmi)
    : FaceAction( dn, ico), _interactor(mmi)
{
    setCheckable(true, false);
    setRespondToEvent( CAMERA_CHANGE, [this](const FVS&){ return this->isChecked();});
}   // end ctor


bool ActionSynchroniseCameraMovement::doAction( FVS& fvs, const QPoint&)
{
    _interactor->disconnect(this);
    if ( isChecked())
    {
        syncActiveCamera( fvs.empty() ? nullptr : fvs.first());
        connect( _interactor, &ModelMoveInteractor::onCameraMove, this, &ActionSynchroniseCameraMovement::syncActiveCamera);
    }   // end if
    return true;
}   // end doAction


// private slot
void ActionSynchroniseCameraMovement::syncActiveCamera( const FV* fv)
{
    ModelViewer* viewer = _interactor->viewer();
    if ( fv)
        viewer = fv->viewer();

    RFeatures::CameraParams cp = viewer->getCamera();
    for ( ModelViewer* v : _viewers)
    {
        if ( v != viewer)   // Set the same camera parameters for v
        {
            v->setCamera(cp);
            v->updateRender();
        }   // end if
    }   // end for
}   // end syncActiveCamera
