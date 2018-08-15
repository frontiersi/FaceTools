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

#include <ActionToggleCameraActorInteraction.h>
#include <FaceModelViewer.h>
using FaceTools::Action::ActionToggleCameraActorInteraction;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


ActionToggleCameraActorInteraction::ActionToggleCameraActorInteraction( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable( true, false);
    // Hijack this action's reportFinished signal to push through affine transform changes on FaceModels to other actions.
    connect( &_interactor, &ModelViewerInteractor::onChangedData, this, &ActionToggleCameraActorInteraction::doOnAffineChange);
}   // end ctor


bool ActionToggleCameraActorInteraction::doAction( FaceControlSet&, const QPoint&)
{
    _interactor.setMoveModels( isChecked());
    return true;
}   // end doAction


void ActionToggleCameraActorInteraction::doOnAffineChange( const FaceControl* fc)
{
    ChangeEventSet cset;
    cset.insert(AFFINE_CHANGE);
    FaceControlSet fcs;
    fcs.insert(const_cast<FaceControl*>(fc));
    emit reportFinished( cset, fcs, true);
}   // end doOnAffineChange
