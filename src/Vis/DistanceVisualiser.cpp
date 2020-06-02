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

#include <Vis/DistanceVisualiser.h>
#include <Metric/DistanceMetricType.h>
#include <FaceModel.h>
using FaceTools::Vis::DistanceVisualiser;
using FaceTools::Vis::FV;


void DistanceVisualiser::doApply( const FV *fv)
{
    const Metric::DistanceMetricType *dmetric = static_cast<const Metric::DistanceMetricType*>(metric());
    const std::vector<Metric::DistMeasure> &dinfo = dmetric->distInfo(fv->data());
    for ( size_t i = 0; i < dinfo.size(); ++i)
    {
        LineView *lv = new LineView;
        lv->setColour( 0.7, 0.0, 0.1);
        _views[fv].push_back(lv);
    }   // end for
}   // end doApply


void DistanceVisualiser::doRefresh( const FV *fv)
{
    const std::vector<LineView*> &lviews = _views.at(fv);
    const Metric::DistanceMetricType *dmetric = static_cast<const Metric::DistanceMetricType*>(metric());
    const std::vector<Metric::DistMeasure> &dinfo = dmetric->distInfo(fv->data());
    for ( size_t i = 0; i < dinfo.size(); ++i)
    {
        const Metric::DistMeasure &dm = dinfo.at(i);
        lviews[i]->update( dm.point0, dm.point1);
    }   // end for
}   // end doRefresh
