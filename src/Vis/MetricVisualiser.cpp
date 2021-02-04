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

#include <Vis/MetricVisualiser.h>
#include <Metric/MetricType.h>
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using FaceTools::Metric::MetricType;


MetricVisualiser::MetricVisualiser() : _metric(nullptr)
{}   // end ctor


void MetricVisualiser::setMetric( const MetricType *m) { _metric = m;}


bool MetricVisualiser::isAvailable( const FV *fv) const
{
    return metric()->hasMeasurement(fv->data());
}   // end isAvailable
