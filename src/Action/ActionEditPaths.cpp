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

#include <Action/ActionEditPaths.h>
#include <Interactor/PathsHandler.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::PathsHandler;
using MS = FaceTools::Action::ModelSelector;


ActionEditPaths::ActionEditPaths( const QString& dn, const QIcon& ic, const QKeySequence& ks)
    : ActionVisualise( dn, ic, &MS::handler<PathsHandler>()->visualisation(), ks), _tchanged(false)
{
    const PathsHandler *h = MS::handler<PathsHandler>();
    connect( h, &PathsHandler::onStartedDrag, this, &ActionEditPaths::_doOnStartedDrag);
    connect( h, &PathsHandler::onFinishedDrag, this, &ActionEditPaths::_doOnFinishedDrag);
    connect( h, &PathsHandler::onEnterHandle, this, &ActionEditPaths::_doOnEnterHandle);
    connect( h, &PathsHandler::onLeaveHandle, this, &ActionEditPaths::_doOnLeaveHandle);
    addRefreshEvent( Event::PATHS_CHANGE);
    addTriggerEvent( Event::RESTORE_CHANGE);
}   // end ctor


bool ActionEditPaths::update( Event e)
{
    bool chk = isChecked();
    const FM *fm = MS::selectedModel();
    if ( fm && isTriggerEvent(e) && has( e, Event::PATHS_CHANGE))
        chk = !fm->currentPaths().empty();
    else
        chk = ActionVisualise::update(e);
    return chk;
}   // end update


Event ActionEditPaths::doAfterAction( Event e)
{
    if (!isTriggerEvent(e))
    {
        if ( isChecked())
            MS::showStatus( "Move path handles by left-clicking and dragging; right click to rename/delete.");
        else
            MS::clearStatus();
    }   // end if
    return ActionVisualise::doAfterAction( e);
}   // end doAfterAction


void ActionEditPaths::_setTempTransparency( bool enable)
{
    FaceTools::Vis::FV *fv = MS::selectedView();
    if ( enable && fv->opacity() == 1.0f)
    {
        fv->setOpacity( 0.99f);
        _tchanged = true;
    }   // end if
    else if ( !enable && _tchanged)
    {
        fv->setOpacity( 1.00f);
        _tchanged = false;
    }   // end else if
}   // end _setTempTransparency


void ActionEditPaths::_doOnStartedDrag( int pid, int hid)
{
    _setTempTransparency( true);
    storeUndo( this, Event::PATHS_CHANGE);
    emit onEvent( Event::PATHS_CHANGE);
}   // end _doOnStartedDrag


void ActionEditPaths::_doOnFinishedDrag( int, int)
{
    _setTempTransparency( false);
    emit onEvent( Event::PATHS_CHANGE);
}   // end _doOnFinishedDrag


void ActionEditPaths::_doOnEnterHandle( int, int hid)
{
    if ( hid == 2)
        _setTempTransparency( true);
}   // end _doOnEnterHandle


void ActionEditPaths::_doOnLeaveHandle( int, int hid)
{
    if ( hid == 2 && !MS::handler<PathsHandler>()->isDragging())
        _setTempTransparency( false);
}   // end _doOnLeaveHandle
