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

#include <Metric/RegionMetricType.h>
using FaceTools::Metric::RegionMetricType;
using FaceTools::Metric::RegionMeasure;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::Vec3f;
using FaceTools::FM;


RegionMetricType::RegionMetricType() : _inPlane(false)
{
    _vis.setMetric(this);
}   // end ctor


namespace {
void setProjectedPoints( RegionMeasure &rm, const std::vector<Vec3f> &pts, const Vec3f &u)
{
    Vec3f mp = Vec3f::Zero();   // Get the mean point
    for ( const Vec3f &p : pts)
        mp += p;
    mp /= pts.size();

    // Project points into the plane defined by point mp and vector u
    int i = 0;
    for ( const Vec3f &p : pts)
        rm.points[i++] = p - (p-mp).dot(u) * u;
}   // end setProjectedPoints
}   // end namespace


float RegionMetricType::update( size_t k, const FM *fm, const std::vector<Vec3f> &ipts, Vec3f, Vec3f nrm, bool, bool inPlane)
{
    // Cache data
    _regionInfo[fm].resize( std::max( _regionInfo[fm].size(), k+1));
    RegionMeasure &rm = _regionInfo[fm][k];
    if ( _inPlane || inPlane)
        setProjectedPoints( rm, ipts, nrm);
    else
        rm.points = ipts;

    const std::vector<Vec3f> &pts = rm.points;
    Vec3f pp = *pts.rbegin();   // Previous point (end of list)
    const Vec3f fv = pp; // Final point

    float area = 0;
    float perim = 0;
    for ( const Vec3f& tp : pts)
    {
        perim += (tp - pp).norm();
        area += r3d::calcArea( pp, tp, fv);
        pp = tp;
    }   // end for

    // Untransform for visualisation
    const Mat4f &iT = fm->inverseTransformMatrix();
    for ( size_t i = 0; i < rm.points.size(); ++i)
        rm.points[i] = r3d::transform( iT, rm.points[i]);

    return area > 0 ? pow(perim,2)/area : 0;
}   // end update
