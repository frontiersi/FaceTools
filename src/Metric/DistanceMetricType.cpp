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

#include <Metric/DistanceMetricType.h>
using FaceTools::Metric::DistanceMetricType;
using FaceTools::Metric::DistMeasure;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::Vec3f;
using FaceTools::FM;


DistanceMetricType::DistanceMetricType() : _inPlane(false)
{
    _vis.setMetric(this);
}   // end ctor


namespace {
void setProjectedPoints( DistMeasure &dm, const std::vector<Vec3f> &pts, const Vec3f &u)
{
    const Vec3f mp = (pts[0] + pts[1]) / 2;
    dm.point0 = pts[0] - (pts[0] - mp).dot(u) * u;
    dm.point1 = pts[1] - (pts[1] - mp).dot(u) * u;
}   // end setProjectedPoints
}   // end namespace


float DistanceMetricType::update( size_t k, const FM *fm, const std::vector<Vec3f>& pts, Vec3f, Vec3f u, bool, bool inPlane)
{
    assert( pts.size() == 2);
    _distInfo[fm].resize( std::max( _distInfo[fm].size(), k+1));
    DistMeasure &dm = _distInfo[fm][k];

    if ( _inPlane || inPlane)
        setProjectedPoints( dm, pts, u);
    else
    {
        dm.point0 = pts[0];
        dm.point1 = pts[1];
    }   // end else

    // Untransform for visualisation
    const Mat4f &iT = fm->inverseTransformMatrix();
    dm.point0 = r3d::transform( iT, dm.point0);
    dm.point1 = r3d::transform( iT, dm.point1);

    return (dm.point0 - dm.point1).norm();
}   // end update
