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

#include <ActionToggleCameraActorInteraction.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Action::ActionToggleCameraActorInteraction;
using FaceTools::Interactor::ActorMoveHandler;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FaceModelViewer;
using FaceTools::ModelViewer;
using FaceTools::FM;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::Interactor::MVI;
using MS = FaceTools::Action::ModelSelector;


ActionToggleCameraActorInteraction::ActionToggleCameraActorInteraction( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    _moveHandler = std::shared_ptr<ActorMoveHandler>( new ActorMoveHandler);
    connect( &*_moveHandler, &ActorMoveHandler::onActorStart, this, &ActionToggleCameraActorInteraction::doOnActorStart);
    connect( &*_moveHandler, &ActorMoveHandler::onActorStop, this, &ActionToggleCameraActorInteraction::doOnActorStop);
    setCheckable( true, false);
}   // end ctor


QString ActionToggleCameraActorInteraction::toolTip() const
{
    return "When on, click and drag the selected model to change its position or orientation.";
}   // end toolTip


QString ActionToggleCameraActorInteraction::whatsThis() const
{
    QStringList htext;
    htext << "With this option toggled off, mouse clicking and dragging causes the camera to move around.";
    htext << "When this option is toggled on, clicking and dragging on a model will reposition or reorient it in space.";
    htext << "Click and drag with the left mouse button to rotate the model in place.";
    htext << "Click and drag with the right mouse button (or hold down the SHIFT key while left clicking and dragging)";
    htext << "to shift the model laterally. Click and drag with the middle mouse button (or hold down the CTRL key while";
    htext << "left or right clicking and dragging) to move the model towards or away from you.";
    htext << "Note that clicking and dragging off the model's surface will still move the camera around, but that this also";
    htext << "toggles this option off (any camera action from the menu/toolbar will also toggle this option off).";
    return tr( htext.join(" ").toStdString().c_str());
}   // end whatsThis


bool ActionToggleCameraActorInteraction::checkState( Event)
{
    const FM* fm = MS::selectedModel();
    // Always disable actor moving if the selected model has more than 1 view
    if ( fm && fm->fvs().size() > 1)
        MS::setInteractionMode(IMode::CAMERA_INTERACTION);
    // Sync the move handler
    _moveHandler->setEnabled( MS::interactionMode() == IMode::ACTOR_INTERACTION);
    return _moveHandler->isEnabled();
}   // end checkState


bool ActionToggleCameraActorInteraction::checkEnable( Event)
{
    const FM* fm = MS::selectedModel();
    return (fm && fm->fvs().size() == 1) || isChecked();
}   // end checkEnabled


void ActionToggleCameraActorInteraction::doAction( Event)
{
    if ( isChecked())
    {
        MS::showStatus( "Model interaction ACTIVE");
        MS::setInteractionMode( IMode::ACTOR_INTERACTION, true);
    }   // end if
    else
    {
        MS::showStatus( "Camera interaction ACTIVE", 5000);
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    }   // end else
}   // end doAction


void ActionToggleCameraActorInteraction::doOnActorStart()
{
    storeUndo( this, Event::AFFINE_CHANGE);
}   // end doOnActorStart


void ActionToggleCameraActorInteraction::doOnActorStop()
{
    //std::cerr << "Stopped moving actor" << std::endl;
    emit onEvent( Event::AFFINE_CHANGE);
}   // end doOnActorStop

