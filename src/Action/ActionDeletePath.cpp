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

#include <Action/ActionDeletePath.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionDeletePath;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::PathsHandler;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionDeletePath::ActionDeletePath( const QString& dn, const QIcon& ico, PathsHandler::Ptr handler)
    : FaceAction( dn, ico), _handler(handler)
{
    connect( &*_handler, &PathsHandler::onLeavePath, this, &ActionDeletePath::doOnLeavePath);
    connect( &*_handler, &PathsHandler::onEnterPath, this, &ActionDeletePath::doOnEnterPath);
}   // end ctor


bool ActionDeletePath::checkEnable( Event)
{
    const FV* fv = MS::cursorView();
    if ( MS::interactionMode() == IMode::ACTOR_INTERACTION || !fv)
        return false;
    return fv == MS::selectedView() && _handler->hoverPath();
}   // end checkEnabled


void ActionDeletePath::doAction( Event)
{
    storeUndo(this, Event::PATHS_CHANGE);

    FV* fv = MS::selectedView();
    const int pid = _handler->hoverPath()->pathId();
    assert(pid >= 0);
    _handler->erasePath( fv, pid);
    emit onEvent( Event::PATHS_CHANGE);
}   // end doAction


void ActionDeletePath::doOnLeavePath()
{
    setLocked(true);
    refreshState();
}   // end doOnLeavePath


void ActionDeletePath::doOnEnterPath()
{
    setLocked(false);
    refreshState();
}   // end doOnEnterPath

