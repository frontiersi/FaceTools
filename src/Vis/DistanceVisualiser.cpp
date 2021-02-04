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

#include <Vis/DistanceVisualiser.h>
#include <Metric/DistanceMetricType.h>
#include <FaceModel.h>
using FaceTools::Vis::DistanceVisualiser;
using FaceTools::Vis::FV;


void DistanceVisualiser::refresh( FV *fv)
{
    const Metric::DistanceMetricType *dmetric = static_cast<const Metric::DistanceMetricType*>(metric());
    const std::vector<Metric::DistMeasure> &dinfo = dmetric->distInfo(fv->data());
    std::vector<LineView> &views = _views[fv];
    views.resize( dinfo.size());
    for ( size_t i = 0; i < dinfo.size(); ++i)
    {
        const Metric::DistMeasure &dm = dinfo.at(i);
        views[i].setColour( 0.7, 0.0, 0.1);
        views[i].update( dm.point0, dm.point1);
    }   // end for
}   // end refresh
