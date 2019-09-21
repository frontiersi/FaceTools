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

#include <Action/ActionUpdateMeasurements.h>
#include <Metric/MetricCalculatorManager.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
#include <QDebug>
#include <algorithm>
using FaceTools::Action::ActionUpdateMeasurements;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Metric::MC;
using MS = FaceTools::Action::ModelSelector;


ActionUpdateMeasurements::ActionUpdateMeasurements()
{
    setAsync( true);
    addTriggerEvent( Event::LANDMARKS_CHANGE);
    addTriggerEvent( Event::SURFACE_DATA_CHANGE);
}   // end ctor


// Ready if at least one metric can be calculated
bool ActionUpdateMeasurements::checkEnable( Event)
{
    const FM* fm = MS::selectedModel();
    if ( fm)
    {
        for ( MC::Ptr mc : MCM::metrics())
        {
            fm->lockForRead();
            const bool cando = mc->canMeasure(fm);
            fm->unlock();
            if ( cando)
                return true;
        }   // end for
    }   // end if
    return false;
}   // end checkEnabled


void ActionUpdateMeasurements::doAction( Event)
{
    FM *fm = MS::selectedModel();
    fm->lockForWrite();

    //std::cerr << "ActionUpdateMeasurements triggered by " << EventGroup(e).name() << std::endl;
    for ( MC::Ptr mc : MCM::metrics())
    {
        if ( mc->canMeasure(fm))
        {
            //std::cerr << "Measuring " << mc->name().toStdString() << std::endl;
            mc->measure(fm);
        }   // end if
    }   // end for

    fm->unlock();
}   // end doAction


void ActionUpdateMeasurements::doAfterAction( Event)
{
    emit onEvent( Event::METRICS_CHANGE);
}   // end doAfterAction
