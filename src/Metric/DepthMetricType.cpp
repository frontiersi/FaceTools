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

#include <Metric/DepthMetricType.h>
#include <r3d/DirectedSurfacePointFinder.h>
using FaceTools::Metric::DepthMetricType;
using FaceTools::Vec3f;
using FaceTools::FM;


DepthMetricType::DepthMetricType() { _vis.setMetric(this);}


float DepthMetricType::update( size_t k, const FM *fm, const std::vector<Vec3f>& pts,
                               Vec3f, Vec3f nv, bool, bool inPlane)
{
    assert( pts.size() >= 1 && pts.size() < 4);
    const Vec3f mp = pts[0];  // Point to measure from
    Vec3f sp = mp;

    // nv will be 0,0,1 for Z (pointing toward camera)
    Vec3f dvec = -nv; // Direction vector to take depth measurement in (defaults to in-plane)

    if ( pts.size() == 2)
    {
        if ( inPlane) // projection along the given in-plane vector from the measuring point
            sp = (pts[1] - mp).dot(dvec) * dvec + mp;
        else
            sp = pts[1];
    }   // end if
    else
    {
        if ( !inPlane && pts.size() == 3)
        {
            const Vec3f lseg = pts[2] - pts[1]; // Metric defined line segment vector
            const Vec3f rvec = lseg.cross(nv);  // Right facing vector
            dvec = lseg.cross( rvec); // Direction to measure depth to surface in from measurement point
            dvec.normalize();
        }   // end if

        const int ifid = r3d::DirectedSurfacePointFinder( fm->kdtree()).find( mp, dvec, sp);
        if ( ifid < 0)
            sp = mp;
    }   // end else

    // Update cached values - note that all are stored untransformed for visualisation
    _depthInfo[fm].resize( std::max( _depthInfo[fm].size(), k+1));
    DepthMeasure &dm = _depthInfo[fm][k];
    // Untransform for visualisation
    const Mat4f iT = fm->inverseTransformMatrix();
    dm.p0 = r3d::transform( iT, mp);
    dm.p1 = r3d::transform( iT, sp);
    return (sp - mp).norm();
}   // end update
