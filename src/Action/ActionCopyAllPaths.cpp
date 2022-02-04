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

#include <Action/ActionCopyAllPaths.h>
#include <Interactor/PathsHandler.h>
#include <FaceModel.h>
using FaceTools::FM;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::ActionCopyAllPaths;
using FaceTools::Interactor::PathsHandler;
using MS = FaceTools::ModelSelect;


ActionCopyAllPaths::ActionCopyAllPaths( const QString& dn, const QIcon& ico) : FaceAction( dn, ico)
{
    addRefreshEvent( Event::PATHS_CHANGE);
}   // end ctor


QString ActionCopyAllPaths::toolTip() const
{
    return tr("Copy all user measurements from the selected model to the other.");
}   // end toolTip


QString ActionCopyAllPaths::whatsThis() const
{
    return tr("Copy all user measurements from the selected model to the other. Uses barycentric coordinate mapping between the two models - both of which must be coregistered against the same underlying anthropometric mask.");
}   // end whatsThis


bool ActionCopyAllPaths::canCopy( const FM *sfm, const FM *dfm)
{
    return dfm && sfm && dfm->hasMask() && sfm->hasMask() && dfm->maskHash() == sfm->maskHash();
}   // end canCopy


bool ActionCopyAllPaths::isAllowed( Event)
{
    FM::RPtr sfm = MS::selectedModelScopedRead();
    FM::RPtr dfm = MS::otherModelScopedRead();
    return canCopy( sfm.get(), dfm.get());
}   // end isAllowed


size_t ActionCopyAllPaths::copy( const FM &sfm, FM &dfm)
{
    const PathSet &pset = sfm.currentPaths();
    for ( int pid : pset.ids())
    {
        Path dpath = pset.path( pid).mapSrcToDst( &sfm, &dfm);
        dfm.addPath( std::move(dpath));
    }   // end for
    return pset.size();
}   // end copy


void ActionCopyAllPaths::doAction( Event)
{
    FM::RPtr sfm = MS::selectedModelScopedRead();   // Source
    FM::WPtr dfm = MS::otherModelScopedWrite();     // Destination
    // TODO storeUndo on other model!
    //storeUndo( this, Event::PATHS_CHANGE);
    copy( *sfm, *dfm);
    PathsHandler *handler = MS::handler<PathsHandler>();
    assert( handler);
    for ( Vis::FV *dfv : dfm->fvs())
        dfv->apply( &handler->visualisation());
}   // end doAction


Event ActionCopyAllPaths::doAfterAction( Event) { return Event::PATHS_CHANGE;}
