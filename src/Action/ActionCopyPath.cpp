/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionCopyPath.h>
#include <Action/ActionCopyAllPaths.h>
#include <Interactor/PathsHandler.h>
#include <FaceModel.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::ActionCopyPath;
using FaceTools::Interactor::PathsHandler;
using MS = FaceTools::ModelSelect;


ActionCopyPath::ActionCopyPath( const QString& dn, const QIcon& ico) : FaceAction( dn, ico)
{
    const PathsHandler *h = MS::handler<PathsHandler>();
    connect( h, &PathsHandler::onEnterHandle, [this](){ this->refresh();});
    connect( h, &PathsHandler::onLeaveHandle, [this](){ this->refresh();});
    addRefreshEvent( Event::PATHS_CHANGE);
}   // end ctor


QString ActionCopyPath::toolTip() const
{
    return tr("Copy the selected user measurement to the other model.");
}   // end toolTip


QString ActionCopyPath::whatsThis() const
{
    return tr("Copy the selected user measurement to the other model using barycentric coordinate mapping to the underlying anthropometric mask. Both the source and destination models must be coregistered against the same anthropometric mask.");
}   // end whatsThis


bool ActionCopyPath::isAllowed( Event)
{
    bool canCopy = false;
    FM::RPtr sfm = MS::selectedModelScopedRead();
    FM::RPtr dfm = MS::otherModelScopedRead();
    if ( ActionCopyAllPaths::canCopy( sfm.get(), dfm.get()))
    {
        const PathsHandler *h = MS::handler<PathsHandler>();
        canCopy = h->hoverPath() && MS::interactionMode() == IMode::CAMERA_INTERACTION;
    }   // end if
    return canCopy;
}   // end isAllowed


void ActionCopyPath::doAction( Event)
{
    PathsHandler *handler = MS::handler<PathsHandler>();
    assert( handler->hoverPath());
    const int pid = handler->hoverPath()->pathId();

    FM::RPtr sfm = MS::selectedModelScopedRead();   // Source
    FM::WPtr dfm = MS::otherModelScopedWrite();     // Destination
    const Path &spath = sfm->currentPaths().path( pid);
    Path dpath = spath.mapSrcToDst( sfm.get(), dfm.get());

    // TODO storeUndo on other model!
    //storeUndo( this, Event::PATHS_CHANGE);
    dfm->addPath( std::move(dpath));
    for ( Vis::FV *dfv : dfm->fvs())
        dfv->apply( &handler->visualisation());
}   // end doAction


Event ActionCopyPath::doAfterAction( Event) { return Event::PATHS_CHANGE;}
