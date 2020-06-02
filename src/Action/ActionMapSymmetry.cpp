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

#include <Action/ActionMapSymmetry.h>
#include <FaceModelSymmetry.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionMapSymmetry;
using FaceTools::Action::Event;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


ActionMapSymmetry::ActionMapSymmetry() : FaceAction( "Map Symmetry")
{
    addPurgeEvent( Event::MESH_CHANGE | Event::MASK_CHANGE);
    addTriggerEvent( Event::MESH_CHANGE | Event::MASK_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionMapSymmetry::isAllowed( Event)
{
    const FM *fm = MS::selectedModel();
    return fm && fm->hasMask();
}   // end isAllowed


void ActionMapSymmetry::doAction( Event)
{
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    FaceModelSymmetry::purge( fm);
    FaceModelSymmetry::add( fm);
    fm->unlock();
}   // end doAction


Event ActionMapSymmetry::doAfterAction( Event) { return Event::SURFACE_DATA_CHANGE;}


void ActionMapSymmetry::purge( const FM* fm) { FaceModelSymmetry::purge(fm);}
