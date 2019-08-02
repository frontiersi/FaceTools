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

#include <LineAngleMetricCalculatorType.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <sstream>
using FaceTools::Metric::LineAngleMetricCalculatorType;
using FaceTools::Metric::MCT;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::FM;


LineAngleMetricCalculatorType::LineAngleMetricCalculatorType( int id, const LmkList* l0, const LmkList* l1)
    : _vis( id, l0, l1) {}


MCT* LineAngleMetricCalculatorType::make( int id, const LmkList* l0, const LmkList* l1) const
{
    return new LineAngleMetricCalculatorType(id, l0, l1);
}   // end make


bool LineAngleMetricCalculatorType::canCalculate( const FM* fm, int aid, const LmkList* ll) const
{
    using SLmk = FaceTools::Landmark::SpecificLandmark;
    const LandmarkSet& lmks = fm->assessment(aid)->landmarks();
    return std::all_of( std::begin(*ll), std::end(*ll), [&lmks]( const SLmk& p){ return lmks.has(p);});
}   // end canCalculate


void LineAngleMetricCalculatorType::measure( std::vector<double>& dvals, const FM* fm, int aid, const LmkList* ll, bool) const
{
    assert( canCalculate( fm, aid, ll));
    const LandmarkSet& lmks = fm->assessment(aid)->landmarks();
    cv::Vec3d p0 = lmks.pos( ll->front());
    cv::Vec3d p1 = lmks.pos( ll->back());
    cv::Vec3d cp;
    if ( ll->size() > 2)
        cp = lmks.pos( ll->at(1));
    else
    {
        const cv::Vec3f mp( 0.5 * (p0 + p1));
        cp = FaceTools::toSurface( fm, mp);
    }   // end else

    // Return a value in the range [0,pi]
    dvals.resize(1);
    dvals[0] = 0.5 * CV_PI * (1.0 - RFeatures::cosi( cp, p0, p1));
}   // end measure
