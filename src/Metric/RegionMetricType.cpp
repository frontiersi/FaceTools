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
void projectToPlane( std::vector<Vec3f> &ops, const std::vector<Vec3f> &pts, const Vec3f &u)
{
    Vec3f mp = Vec3f::Zero();   // Get the mean point
    for ( const Vec3f &p : pts)
        mp += p;
    mp /= pts.size();
    // Project points into the plane defined by point mp and vector u
    ops.resize( pts.size());
    int i = 0;
    for ( const Vec3f &p : pts)
        ops[i++] = p - (p-mp).dot(u) * u;
}   // end projectToPlane
}   // end namespace


// The points define triangles to sum over. The triangles should define a 2D manifold and so the perimeter
// is defined by just those edges that are used once.
float RegionMetricType::update( size_t k, const FM *fm, const std::vector<Vec3f> &ipts, Vec3f, Vec3f nrm, bool, bool inPlane)
{
    // First project the points to the plane if required
    assert( ipts.size() % 3 == 0);
    const std::vector<Vec3f> *cpts = &ipts;
    std::vector<Vec3f> ppts;
    if ( _inPlane || inPlane)
    {
        projectToPlane( ppts, ipts, nrm);
        cpts = &ppts;
    }   // end if
    const std::vector<Vec3f> &pts = *cpts;

    // Add to mesh so we can get the boundary edges using r3d::Boundary,
    // along the way getting the sum of the triangles making the region.
    float area = 0;
    r3d::Mesh regMesh;
    const size_t nts = pts.size() / 3; // Num triangles
    for ( size_t i = 0; i < nts; ++i)
    {
        const int v0 = regMesh.addVertex( pts[3*i]);
        const int v1 = regMesh.addVertex( pts[3*i+1]);
        const int v2 = regMesh.addVertex( pts[3*i+2]);
        const int fid = regMesh.addFace( v0, v1, v2);
        area += regMesh.calcFaceArea( fid);
    }   // end for

    // Get the sorted boundary as a vector of Vec3f
    const IntSet eids = regMesh.pseudoBoundaries( regMesh.faces());
    r3d::Boundaries bnds;
    bnds.sort( regMesh, eids);
    assert( bnds.count() == 1);
    const std::list<int> &blist = bnds.boundary(0);  
    const size_t nperim = blist.size();

    // Copy the ordered boundary vertices into the RegionMeasure struct,
    // untransforming them from the model transform along the way.
    const Mat4f &iT = fm->inverseTransformMatrix();
    _regionInfo[fm].resize( std::max( _regionInfo[fm].size(), k+1));
    RegionMeasure &rm = _regionInfo[fm][k];
    rm.points.resize(nperim);
    int i = 0;
    for ( const int vidx : blist)
        rm.points[i++] = r3d::transform( iT, regMesh.uvtx(vidx));

    // Calculate the total perimeter length
    float perim = 0;
    size_t pi = nperim - 1;   // Previous point (end of list)
    for ( size_t i = 0; i < nperim; ++i)
    {
        const Vec3f &tp = rm.points[i];
        const Vec3f &pp = rm.points[pi];
        perim += (tp - pp).norm();
        pi = i;
    }   // end for

    return area > 0 ? pow(perim,2)/area : 0;
}   // end update
