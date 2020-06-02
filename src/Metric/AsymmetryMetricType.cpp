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

#include <Metric/AsymmetryMetricType.h>
using FaceTools::Metric::AsymmetryMetricType;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::Vec3f;
using FaceTools::FM;


AsymmetryMetricType::AsymmetryMetricType()
{
    _vis.setMetric(this);
}   // end ctor


QString AsymmetryMetricType::typeRemarks() const
{
    QStringList rmks;
    rmks << "Asymmetry is measured as signed distance in the X, Y, and Z axes representing asymmetry through";
    rmks << "the medial, transverse, and coronal planes respectively, and as overall magnitude.";
    rmks << "For a bi-lateral landmark pair {p,q} with p on the right lateral and q on the left,";
    rmks << "calculation is as follows: p is reflected through the medial plane to point p'. Difference";
    rmks << "vector d = q - p' is taken, the components of which correspond to asymmetry along the respective";
    rmks << "axes. Magnitude is as the L2 norm of d. Because the values corresponding to the axial components are";
    rmks << "signed, positive values denote outgrowth in the positive halfspace (or ingrowth in the negative";
    rmks << "halfspace) through the respective plane.";
    return rmks.join(" ");
}   // end typeRemarks


// m: medial mean (point in medial plane of alignment matrix)
// u: positive unit x vector from alignment matrix
float AsymmetryMetricType::update( size_t k, const FM *fm, const std::vector<Vec3f>& pts, Vec3f m, Vec3f u, bool, bool)
{
    assert( k >= 0 && k <= 3);
    assert( pts.size() == 2);
    const Vec3f &p = pts[0];
    const Vec3f &q = pts[1];

    // p is on the subject's right lateral, q on the left. Reflect p through the medial plane so it's on the
    // same lateral (left) as point q and call this reflected point p'. We will take the difference vector q-p';
    // the components of which give the four dimension values x,y,z and absolute magnitude. Note that the
    // x,y,z values are signed.

    _asymmInfo[fm].resize( std::max( _asymmInfo[fm].size(), k+1));
    AsymmetryMeasure &am = _asymmInfo[fm][k];

    const Mat4f &iT = fm->inverseTransformMatrix();
    am.point0 = r3d::transform( iT, p);
    am.point1 = r3d::transform( iT, q);
    const float a = (m - p).dot(u);  // a is distance of p to the plane
    const Vec3f pa = p + 2*a*u;    // pa is p' (p reflected through medial plane)
    //am.delta = q - pa; // Difference of mean point with the intersection point
    am.delta = am.point1 - r3d::transform( iT, pa);

    float v = 0.0f;
    if ( k < 3)
        v = Mat4f::Identity().block<3,1>(0,k).dot(am.delta);
    else
        v = am.delta.norm();

    return v;
}   // end update
