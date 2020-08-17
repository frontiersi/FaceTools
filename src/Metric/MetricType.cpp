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

#include <Metric/MetricType.h>
using FaceTools::Metric::MetricType;
using FaceTools::Metric::MetricParams;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::Vec3f;
using FaceTools::FM;


Vec3f MetricType::normal( const FM *fm) const
{
    const Mat4f &aM = fm->transformMatrix();
    assert( !aM.isZero());
    Vec3f nrm;
    if ( normal() == "x")
        nrm = aM.block<3,1>(0,0);
    else if ( normal() == "y")
        nrm = aM.block<3,1>(0,1);
    else if ( normal() == "z")
        nrm = aM.block<3,1>(0,2);
    return nrm;
}   // end normal


void MetricType::setParams( const MetricParams &prms) 
{
    _prms = prms;
    _prms.spoints = _prms.points;
    _lmids.clear();
    for ( auto &vlmks : _prms.spoints)
    {
        for ( auto &lmlst : vlmks)
        {
            for ( auto &sl : lmlst)
            {
                _lmids.insert( sl.id);
                if ( sl.lat != MID)
                    sl.lat = sl.lat == LEFT ? RIGHT : LEFT;
            }   // end for
        }   // end for
    }   // end for
}   // end setParams


const std::vector<LmkList>& MetricType::points( size_t i, bool swapped) const
{
    return swapped ? _swapPoints(i) : _origPoints(i);
}   // end points


void MetricType::measure( std::vector<float> &results, const FM *fm, bool swapSide, bool inPlane)
{
    const Mat4f &T = fm->transformMatrix();
    const Mat4f &iT = fm->inverseTransformMatrix();
    const LandmarkSet &lmks = fm->currentLandmarks();
    assert( !lmks.empty());
    const Vec3f mp = T.block<3,1>(0,3);
    const Vec3f nv = normal( fm);
    const size_t ndims = dimensions();
    results.resize(ndims);
    size_t k = swapSide ? ndims : 0;
    for ( size_t i = 0; i < ndims; ++i)
    {
        const std::vector<LmkList> &lpts = points( i, swapSide);
        const size_t npoints = lpts.size();
        std::vector<Vec3f> vpts( npoints);
        for ( size_t j = 0; j < npoints; ++j)
            vpts[j] = lmks.toPoint( lpts[j], T, iT);
        results[i] = update( k++, fm, vpts, mp, nv, swapSide, inPlane);
    }   // end for
}   // end measure
