/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ActionUpdateMetrics.h>
#include <MetricCalculatorManager.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
using FaceTools::Action::ActionUpdateMetrics;
using FaceTools::Action::FaceAction;
using FaceTools::Metric::MCSet;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;


ActionUpdateMetrics::ActionUpdateMetrics()
{
    setVisible( false);
    setRespondToEvent( GEOMETRY_CHANGE);
    setRespondToEvent( LANDMARKS_ADD);
    setRespondToEvent( LANDMARKS_CHANGE);
    setRespondToEvent( METADATA_CHANGE);
}   // end ctor


// Ready if at least one of the metrics can be calculated
bool ActionUpdateMetrics::testReady( const FV* fv)
{
    const FM* fm = fv->data();
    bool okay = false;
    for ( MC::Ptr mc : MCM::metrics())
    {
        if ( mc->isAvailable(fm))
        {
            okay = true;
            break;
        }   // end if
    }   // end for
    return okay;
}   // end testReady


bool ActionUpdateMetrics::doAction( FVS& fvs, const QPoint&)
{
    for ( FM* fm : fvs.models())
    {
        const MCSet& mcs = MCM::metrics();
        std::for_each( std::begin(mcs), std::end(mcs), [=](MC::Ptr mc){ mc->calculate(fm);});
    }   // end for
    return true;
}   // end doAction
