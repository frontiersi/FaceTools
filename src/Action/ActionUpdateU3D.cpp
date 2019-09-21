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

#include <Action/ActionUpdateU3D.h>
#include <U3DCache.h>
#include <algorithm>
using FaceTools::Action::ActionUpdateU3D;
using FaceTools::Action::Event;
using FaceTools::U3DCache;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


// public
ActionUpdateU3D::ActionUpdateU3D() : FaceAction( "U3D Updater")
{
    addPurgeEvent( Event::GEOMETRY_CHANGE);
    addPurgeEvent( Event::ORIENTATION_CHANGE);
    addTriggerEvent( Event::GEOMETRY_CHANGE);
    addTriggerEvent( Event::ORIENTATION_CHANGE);
    setAsync( true, true);  // Reentrant!
}   // end ctor


bool ActionUpdateU3D::checkEnable( Event)
{
    return U3DCache::isAvailable() && MS::selectedModel();
}   // end checkEnabled


void ActionUpdateU3D::doAction( Event)
{
    U3DCache::refresh( MS::selectedModel(), true);
}   // end doAction


void ActionUpdateU3D::purge( const FM* fm, Event)
{
    U3DCache::purge(fm);
}   // end purge


void ActionUpdateU3D::doAfterAction( Event)
{
    emit onEvent( Event::U3D_MODEL_CHANGE);
}   // end doAfterAction
