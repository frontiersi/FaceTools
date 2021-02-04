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

#include <Metric/AngleMetricType.h>
using FaceTools::Metric::AngleMetricType;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::Vec3f;
using FaceTools::FM;


AngleMetricType::AngleMetricType() { _vis.setMetric(this);}


float AngleMetricType::update( size_t k, const FM *fm, const std::vector<Vec3f>& pts, Vec3f, Vec3f nrm, bool swapped, bool)
{
    if ( swapped)
        nrm = -nrm;

    assert( pts.size() == 3);
    const Vec3f& c = pts[1];

    // Angle measurements are projected into the plane and are always measured at the root (vertex c)
    Vec3f v0 = pts[0] - c;
    Vec3f v1 = pts[2] - c;
    v0 += -v0.dot(nrm) * nrm;
    v1 += -v1.dot(nrm) * nrm;

    const float delta = sqrtf((v0.squaredNorm() + v1.squaredNorm())/2);
    v0.normalize();
    v1.normalize();

    _angleInfo[fm].resize( std::max( _angleInfo[fm].size(), k+1));
    AngleMeasure &am = _angleInfo[fm][k];
    am.centre = c;
    am.normal = nrm;

    // Get the angle between the two vectors in degrees
    am.degrees = acosf( std::min(1.0f, std::max( -1.0f, v0.dot(v1)))) * 180.0f/EIGEN_PI;

    // If the cross product of the two vectors is in the opposite direction to
    // the given norm, then the angle is negative.
    if ( nrm.dot( v0.cross(v1)) < 0.0f)
        am.degrees = -am.degrees;

    am.point0 = c + v0*delta;
    am.point1 = c + v1*delta;

    const Mat4f &iT = fm->inverseTransformMatrix();
    am.point0 = r3d::transform( iT, am.point0);
    am.point1 = r3d::transform( iT, am.point1);
    am.centre = r3d::transform( iT, am.centre);
    am.normal = iT.block<3,3>(0,0) * am.normal;

    return am.degrees;
}   // end update
