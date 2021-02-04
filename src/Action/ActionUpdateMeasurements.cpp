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

#include <Action/ActionUpdateMeasurements.h>
#include <Interactor/LandmarksHandler.h>
#include <Metric/MetricManager.h>
using FaceTools::Action::ActionUpdateMeasurements;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using MM = FaceTools::Metric::MetricManager;
using MC = FaceTools::Metric::Metric;


ActionUpdateMeasurements::ActionUpdateMeasurements() : FaceAction("Update Measurements")
{
    addTriggerEvent( Event::LANDMARKS_CHANGE | Event::STATS_CHANGE);
#ifdef NDEBUG
    setAsync(true);
#endif
}   // end ctor


bool ActionUpdateMeasurements::updateMeasurement( FM *fm , int mid)
{
    const MC *m = MM::cmetric(mid);
    return m->_canMeasure( fm) && m->_measure(fm);
}   // end updateMeasurement


bool ActionUpdateMeasurements::updateAllMeasurements( FM *fm)
{
    bool updated = false;
    if ( fm)
        for ( int mid : MM::ids())
            updated |= updateMeasurement( fm, mid);
    return updated;
}   // end updateAllMeasurements


bool ActionUpdateMeasurements::doBeforeAction( Event e) { return true;}


void ActionUpdateMeasurements::doAction( Event e)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    updateAllMeasurements( fm.get());
}   // end doAction


Event ActionUpdateMeasurements::doAfterAction( Event e) { return Event::METRICS_CHANGE;}
