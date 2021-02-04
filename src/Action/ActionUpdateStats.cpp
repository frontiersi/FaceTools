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

#include <Action/ActionUpdateStats.h>
#include <Metric/StatsManager.h>
using FaceTools::Action::ActionUpdateStats;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using SM = FaceTools::Metric::StatsManager;


ActionUpdateStats::ActionUpdateStats() : FaceAction("Update Stats")
{
    addTriggerEvent( Event::LOADED_MODEL | Event::METADATA_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionUpdateStats::isAllowed( Event) { return MS::isViewSelected();}
bool ActionUpdateStats::doBeforeAction( Event e) { return isAllowed(e);}


void ActionUpdateStats::doAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    SM::updateStatsForModel( *fm);
}   // end doAction

Event ActionUpdateStats::doAfterAction( Event) { return Event::STATS_CHANGE;}

void ActionUpdateStats::purge( const FM *fm) { SM::purge( *fm);}
