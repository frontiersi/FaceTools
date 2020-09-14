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

#include <Action/ActionUpdateGrowthData.h>
#include <Metric/MetricManager.h>
#include <FaceModel.h>
using FaceTools::Action::ActionUpdateGrowthData;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;
using MM = FaceTools::Metric::MetricManager;

bool ActionUpdateGrowthData::s_autoStats(true);


ActionUpdateGrowthData::ActionUpdateGrowthData()
    : FaceAction("Update Growth Data")
{
    addTriggerEvent( Event::MODEL_SELECT | Event::ASSESSMENT_CHANGE);
}   // end ctor


bool ActionUpdateGrowthData::setStatsToModel( const FM* fm)
{
    bool changedGrowthData = false;
    for ( Metric::MC::Ptr mc : MM::metrics())
    {
        Metric::GrowthDataRanker& gdRanker = mc->growthData();
        gdRanker.setCompatible( fm);    // Will be all if fm null (not auto stats)
        if ( fm)    // Leave current growth data alone if making all compatible
        {
            const Metric::GrowthData *gd = gdRanker.findBestMatch( fm);
            if ( gdRanker.current() != gd)
            {
                gdRanker.setCurrent( gd);
                changedGrowthData = true;
            }   // end if
        }   // end if
    }   // end for
    return changedGrowthData;
}   // end setAutoStats


bool ActionUpdateGrowthData::setAutoStats( bool v)
{
    s_autoStats = v;
    const FM *fm = s_autoStats ? MS::selectedModel() : nullptr;
    return setStatsToModel( fm);
}   // end setAutoStats


void ActionUpdateGrowthData::doAction( Event) { setAutoStats( s_autoStats);} // Just update compatability

Event ActionUpdateGrowthData::doAfterAction( Event) { return Event::STATS_CHANGE;}
