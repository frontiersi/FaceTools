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

#include <Action/ActionMapDelta.h>
#include <FaceModelDeltaStore.h>
#include <FaceModel.h>
using FaceTools::Action::ActionMapDelta;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using FMDS = FaceTools::FaceModelDeltaStore;


ActionMapDelta::ActionMapDelta() : FaceAction( "Map Delta")
{
    addTriggerEvent( Event::MESH_CHANGE | Event::MASK_CHANGE | Event::VIEWER_CHANGE);
    addPurgeEvent( Event::MESH_CHANGE | Event::MASK_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionMapDelta::doBeforeAction( Event e)
{
    FM::RPtr tgtfm = MS::selectedModelScopedRead();
    if ( !tgtfm || !tgtfm->hasMask())
        return false;

    FM::RPtr srcfm = MS::otherModelScopedRead();
    if ( !srcfm || !srcfm->hasMask())
        return false;

    // Mask hashes on both models must match
    if ( srcfm->maskHash() != tgtfm->maskHash())
        return false;

    return !FMDS::has( tgtfm.get(), srcfm.get());
}   // end doBeforeAction


void ActionMapDelta::doAction( Event e)
{
    FM::RPtr tgt = MS::selectedModelScopedRead();
    FM::RPtr src = MS::otherModelScopedRead();
    FMDS::add( tgt.get(), src.get());
    FMDS::add( src.get(), tgt.get());
}   // end doAction


Event ActionMapDelta::doAfterAction( Event) { return Event::SURFACE_DATA_CHANGE;}

void ActionMapDelta::purge( const FM *fm) { FMDS::purge(fm);}

