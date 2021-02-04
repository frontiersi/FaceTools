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

#include <Vis/DepthVisualiser.h>
#include <Metric/DepthMetricType.h>
#include <FaceModel.h>
using FaceTools::Vis::DepthVisualiser;
using FaceTools::Vis::FV;


void DepthVisualiser::refresh( FV *fv)
{
    const Metric::DepthMetricType *dmetric = static_cast<const Metric::DepthMetricType*>(metric());
    const std::vector<Metric::DepthMeasure> &dinfo = dmetric->depthInfo(fv->data());
    std::vector<LineView> &views = _views[fv];
    views.resize( dinfo.size());
    for ( size_t i = 0; i < dinfo.size(); ++i)
    {
        const Metric::DepthMeasure &dm = dinfo.at(i);
        views[i].setColour( 0.1, 0.0, 0.7);
        views[i].update( dm.measurePoint, dm.surfacePoint);
    }   // end for
}   // end refresh
