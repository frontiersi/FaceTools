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

#include <Action/ActionUpdateMeasurements.h>
#include <Interactor/LandmarksHandler.h>
#include <Metric/MetricManager.h>
#include <FaceModel.h>
using FaceTools::Action::ActionUpdateMeasurements;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;
using MM = FaceTools::Metric::MetricManager;


ActionUpdateMeasurements::ActionUpdateMeasurements()
    : FaceAction("Update Measurements"), _ev(Event::NONE)
{
    addTriggerEvent( Event::MESH_CHANGE
                   | Event::AFFINE_CHANGE   // needed for rescaling!
                   | Event::RESTORE_CHANGE
                   | Event::STATS_CHANGE);
}   // end ctor


void ActionUpdateMeasurements::postInit()
{
    using Interactor::LandmarksHandler;
    const LandmarksHandler *h = MS::handler<LandmarksHandler>();
    if ( h)
    {
        std::function<void( int, FaceSide)> fn =
            [this]( int lmid, FaceSide fs)
            {
                if ( updateMeasurementsForLandmark( MS::selectedModel(), lmid))
                    emit this->onEvent( Event::METRICS_CHANGE);
            };
        connect( h, &LandmarksHandler::onDoingDrag, fn);
        connect( h, &LandmarksHandler::onFinishedDrag, fn);
    }   // end if
}   // end postInit


bool ActionUpdateMeasurements::updateAllMeasurements( FM *fm)
{
    bool updated = false;
    if ( fm)
        for ( Metric::MC::Ptr mc : MM::metrics())
            if ( mc->canMeasure(fm) && mc->measure(fm))
                updated = true;
    return updated;
}   // end updateAllMeasurements


bool ActionUpdateMeasurements::updateMeasurementsForLandmark( FM *fm, int lmid)
{
    bool updated = false;
    const Metric::MCSet &mset = MM::metricsForLandmark( lmid);
    for ( const auto &m : mset)
        if ( m->canMeasure( fm) && m->measure(fm))
            updated = true;
    return updated;
}   // end updateMeasurementsForLandmark


bool ActionUpdateMeasurements::updateMeasurementsForLandmarks( FM *fm, const IntSet &lmids)
{
    bool updated = false;
    for ( int lmid : lmids)
        updated |= updateMeasurementsForLandmark( fm, lmid);
    return updated;
}   // end updateMeasurementsForLandmarks


void ActionUpdateMeasurements::doAction( Event e)
{
    FM *fm = MS::selectedModel();
    _ev = Event::NONE;
    if ( isTriggerEvent(e) && fm)
    {
        updateAllMeasurements( fm);
        _ev = Event::METRICS_CHANGE;
    }   // end if
}   // end doAction
