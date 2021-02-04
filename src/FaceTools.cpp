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

#include <FaceTools.h>
#include <LndMrk/LandmarksManager.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <r3d/Transformer.h>
#include <r3d/IterativeSurfacePathFinder.h>
#include <r3d/SurfaceGlobalPlanePathFinder.h>
#include <r3d/SurfaceLocalPlanePathFinder.h>
#include <r3d/SurfacePointFinder.h>
#include <r3d/SurfaceCurveFinder.h>
#include <algorithm>
using FaceTools::FM;
using namespace r3d;


namespace {

void updateNormal( const KDTree &kdt, const Vec3f& v0, const Vec3f& v1, Vec3f& nvec)
{
    // Estimate "down" vector from cross product of base vector with current (inaccurate) face normal.
    const Vec3f evec = v1 - v0;
    Vec3f dvec = evec.cross(nvec);
    dvec.normalize();

    // Find reference locations further down the face from v0 and v1 (at about nostril height)
    const float pdelta = 0.5f * evec.norm();
    const Vec3f r0 = FaceTools::toSurface( kdt, v0 + pdelta * dvec);
    const Vec3f r1 = FaceTools::toSurface( kdt, v1 + pdelta * dvec);

    const Vec3f n0 = (r0-v0).cross(r1-v0);
    const Vec3f n1 = (r0-v1).cross(r1-v1);
    nvec = n0 + n1;
    nvec.normalize();
}   // end updateNormal

}   // end namespace


Vec3f FaceTools::findNormal( const KDTree &kdt, const Vec3f& v0, const Vec3f& v1, const Vec3f& invec)
{
    static const float MIN_DELTA = 1e-7f;
    static const int MAX_TRIES = 12;

    Vec3f nvec = invec;
    float delta = MIN_DELTA + 1;
    int tries = 0;
    while ( fabsf(delta) > MIN_DELTA && tries < MAX_TRIES)
    {
        const Vec3f dnvec = nvec;
        updateNormal( kdt, v0, v1, nvec);
        delta = (nvec - dnvec).squaredNorm();
        tries++;
    }   // end while
    return nvec;
}   // end findNormal


float FaceTools::calcFaceCropRadius( const Vec3f& fcentre, const Vec3f& v0, const Vec3f& v1, float G)
{
    return G * (( fcentre - v0).norm() + (fcentre - v1).norm())/2;
}   // end calcFaceCropRadius


void FaceTools::updateRenderers( const FMS& fms)
{
    FMVS fmvs;
    for ( FM* fm : fms)
    {
        for ( Vis::FV* fv : fm->fvs())
            fmvs.insert(fv->viewer());
    }   // end for
    std::for_each( std::begin(fmvs), std::end(fmvs), [](FMV* fmv){ fmv->updateRender();});
}   // end updateRenderers


Vec3f FaceTools::toSurface( const KDTree &kdt, const Vec3f& v)
{
    return SurfacePointFinder( kdt.mesh()).find( v, kdt.find(v));
}   // end toSurface


float FaceTools::barycentricMapSrcToDst( const FM *src, Vec3f v, const FM *dst, Vec3f &ov)
{
    assert( src->hasMask());
    assert( src->maskHash() == dst->maskHash());

    // First ensure the point is given in untransformed position
    //const Mat4f iT = src->inverseTransformMatrix();
    //v = r3d::transform( iT, v)

    // v is on the source model surface. Find it's closest position on the source mask as vsmsk.
    Vec3f vsmsk;
    int fid = -1;
    int vidx = src->maskKDTree().find( v);  // Beginning vertex on source mask to search from
    const float sqdiff = SurfacePointFinder( src->mask()).find( v, vidx, fid, vsmsk);
    if ( fid < 0)   // Ensure valid face ID for the source mask
        fid = *src->mask().faces(vidx).begin();

    const Vec3f bpos = src->mask().toBarycentric( fid, vsmsk);
    const Vec3f vdmsk = dst->mask().fromBarycentric( fid, bpos);
    ov = toSurface( dst->kdtree(), vdmsk);  // Project back to model surface on destination
    return sqdiff;
}   // end barycentricMapSrcToDst


Vec3f FaceTools::toTarget( const KDTree &kdt, const Vec3f& s, const Vec3f& t)
{
    return SurfacePointFinder( kdt.mesh()).find( t, kdt.find(s));
}   // end toTarget


bool FaceTools::findPath( const KDTree& kdt, const Vec3f& p0, const Vec3f& p1, std::list<Vec3f>& pts)
{
    SurfaceCurveFinder scfinder0( kdt);
    SurfaceCurveFinder scfinderR( kdt);
 
    float psum0 = scfinder0.findPath( p0, p1);
    float psumr = scfinderR.findPath( p1, p0);
    if ( psum0 + psumr < 0.0f)
    {
        //std::cerr << " not found!" << std::endl;
        return false;
    }   // end if

    if ( psum0 < 0)
        psum0 = FLT_MAX;
    if ( psumr < 0)
        psumr = FLT_MAX;

    const std::vector<Vec3f>* lpath = &scfinder0.lastPath();
    if ( psumr < psum0)
        lpath = &scfinderR.lastPath();
    assert( lpath);
    pts = std::list<Vec3f>( lpath->begin(), lpath->end());
    if ( psumr < psum0)
        pts.reverse();

    return true;
}   // end findPath


bool FaceTools::findStraightPath( const KDTree &kdt, const Vec3f& p0, const Vec3f& p1, std::list<Vec3f>& pts)
{
    IterativeSurfacePathFinder pfinder( kdt);
    if ( pfinder.findPath( p0, p1) > 0)
    {
        const std::vector<Vec3f>& lpath = pfinder.lastPath();
        pts = std::list<Vec3f>( lpath.begin(), lpath.end());
    }   // end if
    return !pts.empty();
}   // end findStraightPath


bool FaceTools::findCurveFollowingPath( const KDTree &kdt, const Vec3f& p0, const Vec3f& p1, std::list<Vec3f>& pts)
{
    SurfaceLocalPlanePathFinder pfinder( kdt);
    if ( pfinder.findPath( p0, p1) >= 0)
    {
        const std::vector<Vec3f>& lpath = pfinder.lastPath();
        pts = std::list<Vec3f>( lpath.begin(), lpath.end());
    }   // end if
    return !pts.empty();
}   // end findCurveFollowingPath


bool FaceTools::findOrientedPath( const KDTree &kdt, const Vec3f& p0, const Vec3f& p1, const Vec3f &u, std::list<Vec3f>& pts)
{
    SurfaceGlobalPlanePathFinder pfinder( kdt, u);
    if ( pfinder.findPath( p0, p1) >= 0)
    {
        const std::vector<Vec3f>& lpath = pfinder.lastPath();
        pts = std::list<Vec3f>( lpath.begin(), lpath.end());
    }   // end if
    return !pts.empty();
}   // end findOrientedPath


Vec3f FaceTools::findHighOrLowPoint( const KDTree& kdt, const Vec3f& p0, const Vec3f& p1)
{
    const Vec3f u = p1-p0;
    const Vec3f r = u.cross(Vec3f(0,0,1));
    Vec3f dv = r.cross(u);   // Normalise the direction vector for depth
    dv.normalize();

    std::list<Vec3f> pts;
    const bool foundPath = findStraightPath( kdt, p0, p1, pts);
    if ( !foundPath)
        std::cerr << "[ERROR] FaceTools::findHighOrLowPoint: findStraightPath returned false!" << std::endl;

    const Vec3f a = 0.5f * (p1 + p0);
    float maxd = 0;
    Vec3f dp = a;   // Absolute deepest point
    for ( const Vec3f& p : pts)
    {
        const Vec3f pa = p - a;
        const float d = fabsf(pa.dot(dv));  // Absolute deepest (i.e. hill or valley)
        if ( d >= maxd)
        {
            maxd = d;
            dp = p;
        }   // end if
    }   // end for

    return dp;
}   // end findHighOrLowPoint


/*
QColor FaceTools::chooseContrasting( const QColor& a)
{
    QColor b;
    b.setRed( abs(a.red() - 255) > a.red() ? 255 : 0);
    b.setGreen( abs(a.green() - 255) > a.green() ? 255 : 0);
    b.setBlue( abs(a.blue() - 255) > a.blue() ? 255 : 0);
    return b;
}   // end chooseContrasting
*/


QColor FaceTools::chooseContrasting( const QColor& a)
{
    QColor b;
    double nr = 0.299*a.red();
    double ng = 0.587*a.green();
    double nb = 0.114*a.blue();
    if ( (nr + ng + nb) > 186.0)
        b = Qt::black;
    else
        b = Qt::white;
    return b;
}   // end chooseContrasting
