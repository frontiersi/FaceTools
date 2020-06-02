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

#include <Vis/RegionVisualiser.h>
#include <Metric/RegionMetricType.h>
#include <FaceModel.h>
using FaceTools::Vis::RegionVisualiser;
using FaceTools::Vis::FV;


void RegionVisualiser::doApply( const FV *fv)
{
    const Metric::RegionMetricType *rmetric = static_cast<const Metric::RegionMetricType*>(metric());
    const std::vector<Metric::RegionMeasure> &rinfo = rmetric->regionInfo(fv->data());
    for ( size_t i = 0; i < rinfo.size(); ++i)
    {
        LoopView *lv = new LoopView;
        lv->setColour( 0.5, 0.1, 0.5);
        _views[fv].push_back(lv);
    }   // end for
}   // end doApply


void RegionVisualiser::doRefresh( const FV *fv)
{
    const std::vector<LoopView*> &lviews = _views.at(fv);
    const Metric::RegionMetricType *rmetric = static_cast<const Metric::RegionMetricType*>(metric());
    const std::vector<Metric::RegionMeasure> &rinfo = rmetric->regionInfo(fv->data());
    for ( size_t i = 0; i < rinfo.size(); ++i)
    {
        const Metric::RegionMeasure &rm = rinfo.at(i);
        lviews[i]->update( rm.points);
    }   // end for
}   // end doRefresh
