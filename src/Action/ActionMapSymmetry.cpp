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

#include <Action/ActionMapSymmetry.h>
#include <FaceModelSymmetryStore.h>
#include <MaskRegistration.h>
#include <FaceModel.h>
using FaceTools::Action::ActionMapSymmetry;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


ActionMapSymmetry::ActionMapSymmetry() : FaceAction( "Map Symmetry")
{
    addPurgeEvent( Event::MESH_CHANGE | Event::MASK_CHANGE);
    addTriggerEvent( Event::MESH_CHANGE | Event::MASK_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionMapSymmetry::doBeforeAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    return fm && fm->maskHash() == MaskRegistration::maskData()->hash;
}   // end doBeforeAction


void ActionMapSymmetry::doAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    FaceModelSymmetryStore::add( fm.get());
}   // end doAction


Event ActionMapSymmetry::doAfterAction( Event)
{
    return Event::SURFACE_DATA_CHANGE;
}   // end doAfterAction


void ActionMapSymmetry::purge( const FM* fm) { FaceModelSymmetryStore::purge(fm);}
