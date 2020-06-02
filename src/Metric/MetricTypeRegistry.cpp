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

#include <Metric/MetricTypeRegistry.h>
#include <Metric/DistanceMetricType.h>
#include <Metric/AngleMetricType.h>
#include <Metric/AsymmetryMetricType.h>
#include <Metric/DepthMetricType.h>
#include <Metric/RegionMetricType.h>
#include <LndMrk/LandmarksManager.h>
#include <iostream>
using FaceTools::Metric::MetricTypeRegistry;
using FaceTools::Metric::MetricParams;
using FaceTools::Metric::MetricType;


MetricType::Ptr MetricTypeRegistry::make( const QString& acat, const MetricParams &prms)
{
    static const std::string WSTR = "[WARN] FaceTools::Metric::MetricTypeRegistry::make: ";
    if ( Landmark::LandmarksManager::count() == 0)
    {
        std::cerr << WSTR << "Load some landmarks first!" << std::endl;
        return nullptr;
    }   // end if

    const QString cat = acat.toLower();
    MetricType::Ptr m;
    if ( cat == "angle")
        m = MetricType::Ptr( new AngleMetricType);
    else if ( cat == "asymmetry")
        m = MetricType::Ptr( new AsymmetryMetricType);
    else if ( cat == "depth")
        m = MetricType::Ptr( new DepthMetricType);
    else if ( cat == "distance")
        m = MetricType::Ptr( new DistanceMetricType);
    else if ( cat == "region")
        m = MetricType::Ptr( new RegionMetricType);
    else
        std::cerr << WSTR << "Metric category '" << acat.toStdString() << "' unavailable!" << std::endl;

    if ( m)
        m->setParams( prms);
    return m;
}   // end make
