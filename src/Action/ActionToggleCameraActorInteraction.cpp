/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionToggleCameraActorInteraction.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Action::ActionToggleCameraActorInteraction;
using FaceTools::Interactor::MovementNotifier;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FaceModelViewer;
using FaceTools::ModelViewer;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionToggleCameraActorInteraction::ActionToggleCameraActorInteraction( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    connect( &_moveNotifier, &MovementNotifier::onActorStart,
            this, &ActionToggleCameraActorInteraction::_doOnActorStart);
    connect( &_moveNotifier, &MovementNotifier::onActorStop,
            this, &ActionToggleCameraActorInteraction::_doOnActorStop);
    connect( &_moveNotifier, &MovementNotifier::onCameraStart,
            this, &ActionToggleCameraActorInteraction::_doOnCameraMove);
    connect( &_moveNotifier, &MovementNotifier::onCameraMove,
            this, &ActionToggleCameraActorInteraction::_doOnCameraMove);
    connect( &_moveNotifier, &MovementNotifier::onCameraStop,
            this, &ActionToggleCameraActorInteraction::_doOnCameraStop);
    setCheckable( true, false);
    addRefreshEvent( Event::AFFINE_CHANGE | Event::MESH_CHANGE | Event::SAVED_MODEL);
}   // end ctor


QString ActionToggleCameraActorInteraction::toolTip() const
{
    return "Toggle on to reposition models by clicking and dragging on them.";
}   // end toolTip


QString ActionToggleCameraActorInteraction::whatsThis() const
{
    QStringList ht;
    ht << "If toggled on, clicking and dragging on a model repositions it.";
    ht << "Click and drag with the left mouse button to rotate the model in place.";
    ht << "Click and drag with the right mouse button (or hold SHIFT while left clicking and dragging)";
    ht << "to shift the model laterally. Click and drag with the middle mouse button (or hold CTRL while";
    ht << "left or right clicking and dragging) to move the model towards or away from you.";
    ht << "Clicking and dragging outside a model switches back to camera interaction mode.";
    return tr( ht.join(" ").toStdString().c_str());
}   // end whatsThis


bool ActionToggleCameraActorInteraction::update( Event)
{
    return MS::interactionMode() == IMode::ACTOR_INTERACTION;
}   // end update


bool ActionToggleCameraActorInteraction::isAllowed( Event) { return isChecked() || MS::isViewSelected();}


void ActionToggleCameraActorInteraction::doAction( Event)
{
    if ( isChecked())
    {
        MS::showStatus( "Model interaction ACTIVE");
        MS::setInteractionMode( IMode::ACTOR_INTERACTION);
    }   // end if
    else
    {
        MS::showStatus( "Camera interaction ACTIVE", 5000);
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    }   // end else
}   // end doAction


Event ActionToggleCameraActorInteraction::doAfterAction( Event) { return Event::NONE;}


void ActionToggleCameraActorInteraction::_doOnActorStart()
{
    storeUndo( this, Event::AFFINE_CHANGE);
}   // end _doOnActorStart


void ActionToggleCameraActorInteraction::_doOnActorStop()
{
    emit onEvent( Event::AFFINE_CHANGE);
}   // end _doOnActorStop


void ActionToggleCameraActorInteraction::_doOnCameraMove()
{
    emit onEvent( Event::CAMERA_CHANGE);
}   // end _doOnCameraMove


void ActionToggleCameraActorInteraction::_doOnCameraStop()
{
    // Always deactivate actor interaction mode on stopping the camera
    if ( isChecked())
        setChecked(false);
}   // end _doOnCameraStop
