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

#include <Action/ActionMapCurvature.h>
#include <FaceModelCurvature.h>
#include <FaceModel.h>
#include <algorithm>
using FaceTools::Action::ActionMapCurvature;
using FaceTools::Action::Event;
using FaceTools::FaceModelCurvature;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


// public
ActionMapCurvature::ActionMapCurvature() : FaceAction( "Map Curvature")
{
    addPurgeEvent( Event::GEOMETRY_CHANGE);
    addTriggerEvent( Event::GEOMETRY_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionMapCurvature::checkEnable( Event)
{
    const FM* fm = MS::selectedModel();
    return fm && FaceModelCurvature::rmetrics(fm) == nullptr;
}   // end checkEnabled


void ActionMapCurvature::doAction( Event)
{
    const FM* fm = MS::selectedModel();
    fm->lockForRead();
    FaceModelCurvature::add(fm);    // Blocks
    fm->unlock();
}   // end doAction


void ActionMapCurvature::doAfterAction( Event)
{
    emit onEvent( Event::SURFACE_DATA_CHANGE);
}   // end doAfterAction


void ActionMapCurvature::purge( const FM* fm, Event)
{
    FaceModelCurvature::purge(fm);
}   // end purge
