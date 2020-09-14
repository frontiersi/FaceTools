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

#include <Action/ActionDeletePath.h>
#include <Interactor/PathsHandler.h>
#include <FaceModel.h>
using FaceTools::Action::ActionDeletePath;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::PathsHandler;
using MS = FaceTools::Action::ModelSelector;


ActionDeletePath::ActionDeletePath( const QString& dn, const QIcon& ico, const QKeySequence &ks)
    : FaceAction( dn, ico, ks)
{
    const PathsHandler *h = MS::handler<PathsHandler>();
    connect( h, &PathsHandler::onEnterHandle, [this](){ this->refresh();});
    connect( h, &PathsHandler::onLeaveHandle, [this](){ this->refresh();});
    addRefreshEvent( Event::PATHS_CHANGE);
}   // end ctor


bool ActionDeletePath::isAllowed( Event)
{
    const PathsHandler *h = MS::handler<PathsHandler>();
    return MS::interactionMode() == IMode::CAMERA_INTERACTION && h->hoverPath();
}   // end isAllowed


void ActionDeletePath::doAction( Event)
{
    storeUndo(this, Event::PATHS_CHANGE);
    FM *fm = MS::selectedModel();
    PathsHandler *handler = MS::handler<PathsHandler>();
    assert( handler->hoverPath());
    const int pid = handler->leavePath();
    handler->visualisation().erasePath( fm, pid);
    fm->lockForWrite();
    fm->removePath(pid);
    fm->unlock();
}   // end doAction


Event ActionDeletePath::doAfterAction( Event)
{
    MS::showStatus( "Measurement deleted!", 5000);
    return Event::PATHS_CHANGE;
}   // end doAfterAction
