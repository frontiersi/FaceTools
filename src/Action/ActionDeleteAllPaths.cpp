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

#include <Action/ActionDeleteAllPaths.h>
#include <Interactor/PathsHandler.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionDeleteAllPaths;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::PathsHandler;
using MS = FaceTools::Action::ModelSelector;


ActionDeleteAllPaths::ActionDeleteAllPaths( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    addRefreshEvent( Event::PATHS_CHANGE);
}   // end ctor
    

bool ActionDeleteAllPaths::isAllowed( Event)
{
    PathsHandler *h = MS::handler<PathsHandler>();
    const Vis::FV* fv = MS::selectedView();
    return fv && MS::interactionMode() != IMode::ACTOR_INTERACTION
              && fv->data()->currentAssessment()->hasPaths()
              && fv->isApplied( &h->visualisation());
}   // end isAllowed


bool ActionDeleteAllPaths::doBeforeAction( Event)
{
    using QMB = QMessageBox;
    static const QString tit = tr("Delete all?");
    static const QString msg = tr("All custom measurements on this assessment will be erased! Continue?");
    return QMB::Yes == QMB::warning( static_cast<QWidget*>(parent()), tit, msg, QMB::Yes | QMB::No, QMB::Yes);
}   // end doBeforeAction


void ActionDeleteAllPaths::doAction( Event)
{
    storeUndo(this, Event::PATHS_CHANGE);
    FM *fm = MS::selectedModel();

    fm->lockForWrite();

    const IntSet pids = fm->currentAssessment()->paths().ids(); // Copy out because altering
    PathsHandler *handler = MS::handler<PathsHandler>();
    for ( int pid : pids)
    {
        fm->removePath(pid);
        handler->visualisation().erasePath( fm, pid);
    }   // end for

    if ( handler->hoverPath())
        handler->leavePath();

    fm->unlock();
}   // end doAction


Event ActionDeleteAllPaths::doAfterAction( Event)
{
    MS::showStatus( "All measurements deleted!", 5000);
    return Event::PATHS_CHANGE;
}   // end doAfterAction

