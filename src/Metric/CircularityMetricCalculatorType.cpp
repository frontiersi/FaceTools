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

#include <CircularityMetricCalculatorType.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FeatureUtils.h>   // RFeatures
#include <sstream>
using FaceTools::Metric::CircularityMetricCalculatorType;
using FaceTools::Metric::MCT;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::FM;


CircularityMetricCalculatorType::CircularityMetricCalculatorType( int id, const LmkList* l0, const LmkList* l1)
    : _vis( id, l0, l1) {}


MCT* CircularityMetricCalculatorType::make( int id, const LmkList* l0, const LmkList* l1) const
{
    return new CircularityMetricCalculatorType(id, l0, l1);
}   // end make


bool CircularityMetricCalculatorType::canCalculate( const FM* fm, const LmkList* ll) const
{
    using SLmk = FaceTools::Landmark::SpecificLandmark;
    const LandmarkSet& lmks = fm->landmarks();
    return std::all_of( std::begin(*ll), std::end(*ll), [&lmks]( const SLmk& p){ return lmks.has(p);});
}   // end canCalculate


void CircularityMetricCalculatorType::measure( std::vector<double>& dvals, const FM* fm, const LmkList* ll) const
{
    const LandmarkSet& lmks = fm->landmarks();

    const auto* pp = &*ll->rbegin();     // Previous point
    const cv::Vec3f fv = lmks.pos( *pp); // Final point

    double area = 0;
    double perim = 0;
    for ( const auto& tp : *ll)
    {
        const cv::Vec3f v0 = lmks.pos( *pp);
        const cv::Vec3f v1 = lmks.pos( tp);

        perim += cv::norm(v1 - v0);
        area += RFeatures::calcTriangleArea( v0, v1, fv);

        pp = &tp;
    }   // end for

    dvals.resize(1);
    dvals[0] = area > 0 ? pow(perim,2)/area : 0;
}   // end measure
