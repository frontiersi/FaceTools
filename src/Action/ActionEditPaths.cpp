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

#include <Action/ActionEditPaths.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::PathsHandler;
using FaceTools::FM;
using FaceTools::Vis::FV;
using FaceTools::Vis::PathView;
using MS = FaceTools::Action::ModelSelector;

Q_DECLARE_METATYPE( FaceTools::PathSet::Ptr)


ActionEditPaths::ActionEditPaths( const QString& dn, const QIcon& ic, PathsHandler::Ptr handler, const QKeySequence& ks)
    : ActionVisualise( dn, ic, handler->visualisation(), ks), _handler( handler)
{
    connect( &*_handler, &PathsHandler::onStartedDrag, [this](){ storeUndo(this);});
    connect( &*_handler, &PathsHandler::onFinishedDrag, [this](){ emit onEvent( Event::PATHS_CHANGE);});
    addTriggerEvent( Event::PATHS_CHANGE);
}   // end ctor


bool ActionEditPaths::checkState( Event e)
{
    bool chk = ActionVisualise::checkState(e);    // true if _vis->isVisible true
    _handler->setEnabled(chk);
    return chk;
}   // end checkState


void ActionEditPaths::doAction( Event e)
{
    ActionVisualise::doAction(e);
    if ( isChecked())
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        MS::showStatus( "Move path handles by left-clicking and dragging; right click to rename/delete.");
        if ( _handler->hoverPath())  // Ensure captions of hovered over path is up-to-date.
            _handler->setCaption( MS::selectedView(), _handler->hoverPath()->pathId());
    }   // end if
    else
        MS::clearStatus();
}   // end doAction


UndoState::Ptr ActionEditPaths::makeUndoState() const
{
    UndoState::Ptr us = UndoState::create( this, Event::PATHS_CHANGE);
    us->setName( "Move Path Handle");
    PathSet::Ptr paths = us->model()->currentAssessment()->paths().deepCopy();
    us->setUserData( "Paths", QVariant::fromValue(paths));
    return us;
}   // end makeUndoState


void ActionEditPaths::restoreState( const UndoState* us)
{
    PathSet::Ptr paths = us->userData("Paths").value<PathSet::Ptr>();
    FM* fm = us->model();
    fm->setPaths(paths);
    _handler->visualisation()->syncPaths(fm);
}   // end restoreState
