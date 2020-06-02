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

#include <Metric/DepthMetricType.h>
#include <r3d/DirectedSurfacePointFinder.h>
using FaceTools::Metric::DepthMetricType;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::Vec3f;
using FaceTools::FM;


DepthMetricType::DepthMetricType() : _inPlane(false)
{
    _vis.setMetric(this);
}   // end ctor


float DepthMetricType::update( size_t k, const FM *fm, const std::vector<Vec3f>& pts, Vec3f, Vec3f nv, bool, bool inPlane)
{
    assert( pts.size() == 3 || pts.size() == 1);
    Vec3f mp = pts.size() == 1 ? pts[0] : pts[2];    // Point to measure from

    // nv is always positive
    Vec3f dvec; // Direction vector to take depth measurement in
    if ( _inPlane || inPlane || pts.size() == 1)
        dvec = -nv;
    else
    {
        const Vec3f lseg = pts[1] - pts[0]; // Metric defined line segment vector
        const Vec3f rvec = lseg.cross(nv);  // Right facing vector
        dvec = lseg.cross( rvec); // Direction to measure depth to surface in from measurement point
        dvec.normalize();
    }   // end else

    // Update cached values - note that all are stored untransformed for visualisation
    _depthInfo[fm].resize( std::max( _depthInfo[fm].size(), k+1));
    DepthMeasure &dm = _depthInfo[fm][k];
    dm.measurePoint = mp;
    dm.surfacePoint = mp;

    float depth = 0.0f;
    Vec3f sp;
    const int ifid = r3d::DirectedSurfacePointFinder( fm->kdtree()).find( mp, dvec, sp);
    if ( ifid >= 0)
    {
        const Vec3f toSurf = sp - mp;
        depth = toSurf.norm();
        if ( dvec.dot(toSurf) < 0)  // Depth may be measured negatively
            depth = -depth;
        dm.surfacePoint = sp;
    }   // end if

    // Untransform for visualisation
    const Mat4f &iT = fm->inverseTransformMatrix();
    dm.measurePoint = r3d::transform( iT, dm.measurePoint);
    dm.surfacePoint = r3d::transform( iT, dm.surfacePoint);

    return depth;
}   // end update
