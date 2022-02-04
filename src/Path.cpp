/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Path.h>
#include <r3d/Orientation.h>
#include <FaceTools.h>
#include <FaceModel.h>
using FaceTools::Vec3f;
using FaceTools::Mat4f;
using FaceTools::Mat3f;
using FaceTools::Path;
using FaceTools::FM;

namespace {

float _calcAngle( const Vec3f &dh0, const Vec3f &dh1)
{
    float angle = NAN;
    if ( !dh0.isZero() && !dh1.isZero())
        angle = acosf( dh0.dot(dh1)/(dh0.norm() * dh1.norm())) * 180.0f/EIGEN_PI;
    if ( std::isnan(angle))
        angle = 180.0f;
    return angle;
}   // end _calcAngle

}   // end namespace


// static definition
Path::PathType Path::s_pathType( Path::ORIENTED_CURVE);

void Path::setPathType( Path::PathType pt) { s_pathType = pt;}


Path::Path()
    : _id(-1), _name(""), _validPath(false),
      _elen(0), _slen(0), _area(0), _depth(0), _angle(0), _angleS(0), _angleT(0), _angleC(0)
{
    _name = QString("Unnamed_%1").arg(_id).toStdString();
    _vtxs.resize(2);
}   // end ctor


Path::Path( int i, const Vec3f& v)
    : _id(i), _name(""), _validPath(false),
    _elen(0), _slen(0), _area(0), _depth(0), _angle(0), _angleS(0), _angleT(0), _angleC(0)
{
    _name = QString("Unnamed_%1").arg(_id).toStdString();
    _vtxs.resize(2);
    _vtxs.front() = _vtxs.back() = _dhan = v;
}   // end ctor


Path Path::mapSrcToDst( const FM *sfm, const FM *dfm) const
{
    Path pth;
    pth._id = _id;
    pth._name = _name;

    const Mat4f T = dfm->transformMatrix();
    const Mat4f iT = sfm->inverseTransformMatrix();
    pth._orient = T.block<3,3>(0,0) * iT.block<3,3>(0,0) * _orient;
    pth._orient.normalize();

    Vec3f h0, h1, hd;
    barycentricMapSrcToDst( sfm, handle0(), dfm, h0);
    barycentricMapSrcToDst( sfm, handle1(), dfm, h1);
    barycentricMapSrcToDst( sfm, depthHandle(), dfm, hd);
    pth.setHandle0( h0);
    pth.setHandle1( h1);
    pth.setDepthHandle( hd);

    pth.updatePath( dfm);
    pth.updateMeasures( dfm->inverseTransformMatrix().block<3,3>(0,0));
    return pth;
}   // end mapSrcToDst


bool Path::updatePath( const FM* fm)
{
    _validPath = false;
    Vec3f v0 = _vtxs.front();
    Vec3f v1 = _vtxs.back();

    const Vec3f &u = orientation(); // For ORIENTED_CURVE

    if ( v0 == v1)
        _validPath = true;
    else
    {
        // Obtain the path according to preferred method
        _vtxs.clear();
        switch ( s_pathType)
        {
            case CURVE_FOLLOWING_0:
                _validPath = findPath( fm->kdtree(), v0, v1, _vtxs); // Previous
                break;
            case CURVE_FOLLOWING_1:
                //_validPath = findCurveFollowingPath( fm->kdtree(), v0, v1, _vtxs);   // Experimental
                break;
            case STRAIGHT_CURVE:
                //_validPath = findStraightPath( fm->kdtree(), v0, v1, _vtxs);
                break;
            case ORIENTED_CURVE:    // Default
                assert( u != Vec3f::Zero());
                _validPath = findSlicedPath( fm->kdtree(), v0, v1, u, _vtxs);
                break;
        };  // end switch
    }   // end if

    if ( !_validPath)
    {
        _vtxs.resize(2);
        setHandle0(v0);
        setHandle1(v1);
    }   // end else

    assert(_vtxs.size() >= 2);
    return _validPath;
}   // end updatePath


void Path::updateMeasures( const Mat3f &iR)
{
    const Vec3f& h0 = handle0();    // First point in path
    const Vec3f& h1 = handle1();    // Last point in path
    const Vec3f& hd = depthHandle();

    _slen = 0.0f;
    _area = 0.0f;
    _depth = 0.0f;

    Vec3f u = h1 - h0;  // Could be zero vector
    _elen = u.norm();   // May be zero
    if ( _elen > 0.0f)
        u = u / _elen;  // Euclidean direction between end points

    // Find dhd as the projection magnitude of hd-h0 along h1-h0 but bounded by the endpoints.
    const float dhd = std::min( std::max( 0.0f, u.dot(hd-h0)), _elen);
    // Find two vectors va and vb where va = a-h0, and |va| < dhd, and vb = b-h0, and |vb| >= dhd,
    // and a and b are consecutive points in the surface path.

    float pd = 0.0f;
    float po = 0.0f;
    const Vec3f *a = &h0;
    const Vec3f *b = nullptr;
    for ( const Vec3f &v : _vtxs) // Always from handle0 to handle1
    {
        const Vec3f vh0 = v - h0;   // This vertex delta from h0
        const float d = vh0.dot(u); // This vertex baseline delta from h0

        // && !b because it's possible (though unlikely) that the projection
        // will be shorter again after being longer (after b is set).
        if ( d < dhd && !b)
            a = &v;
        else if ( !b)
            b = &v;

        const float a = std::max( 0.0f, d - pd);    // Baseline delta between this and previous vertex (always >= 0)
        const float o = sqrtf( std::max( 0.0f, vh0.squaredNorm() - d*d)); // This vertex height from baseline
        const float b = o - po; // Height (from baseline) delta between this and previous vertex (may be <= 0)
        _area += a * (o + b/2);
        _slen += sqrtf(std::max( 0.0f, a*a + b*b));
        pd = d;
        po = o;
    }   // end for

    if ( !b)
        b = &h1;

    float da = 0.0f;
    if ( a != &h0)
        da = (*a - h0).dot(u);   // Projected distance of a along u
    float db = 0.0f;
    if ( b != &h0)
        db = (*b - h0).dot(u);   // Projected distance of b along u

    // dhd is in [da,db] and vertices a and b are the endpoints of a single path segment.
    // Get the exact point along this segment where depth should be measured from.
    _dsurf = *a;
    if ( a != b)
        _dsurf += (*b-*a) * ((dhd - da) / (db - da));
    _dline = h0 + dhd * u;
    _depth = (_dline - _dsurf).norm();
    _calcAngles( iR);
}   // end updateMeasures


void Path::transform( const Mat4f &t)
{
    for ( Vec3f &v : _vtxs)
        v = r3d::transform( t, v);
    _dhan = r3d::transform( t, _dhan);
    _dsurf = r3d::transform( t, _dsurf);
    _dline = r3d::transform( t, _dline);
    _orient = t.block<3,3>(0,0) * _orient;
    _orient.normalize();
}   // end transform


void Path::setHandle0( const Vec3f& v) { _vtxs.front() = v;}
void Path::setHandle1( const Vec3f& v) { _vtxs.back() = v;}
void Path::setDepthHandle( const Vec3f& v) { _dhan = v;}

void Path::setOrientation( const Vec3f& u)
{
    _orient = u;
    _orient.normalize();
}   // end setOrientation


void Path::setHandle( int h, const Vec3f &v)
{
    switch (h)
    {
        case 0:
            setHandle0(v);
            break;
        case 1:
            setHandle1(v);
            break;
        case 2:
            setDepthHandle(v);
            break;
        default:
            assert(false);
    }   // end switch
}   // end setHandle


const Vec3f& Path::handle( int h) const
{
    assert( h >= 0 && h <= 2);
    const Vec3f *v = &depthHandle();
    switch (h)
    {
        case 0:
            v = &handle0();
            break;
        case 1:
            v = &handle1();
            break;
    }   // end switch
    return *v;
}   // end handle


void Path::write( PTree& pathsNode, bool withFullPath) const
{
    PTree& pnode = pathsNode.add("Path","");
    pnode.put( "Name", name());
    r3d::putNamedVertex( pnode, "V0", handle0());
    r3d::putNamedVertex( pnode, "V1", handle1());
    r3d::putNamedVertex( pnode, "VD", depthHandle());
    r3d::putNamedVertex( pnode, "Orient", orientation());

    PTree& metrics = pnode.add( "Metrics", "");
    metrics.put("DirectDistance", euclideanDistance());
    metrics.put("SurfaceDistance", surfaceDistance());
    metrics.put("Depth", depth());
    metrics.put("Angle", angle());
    metrics.put("AngleS", angleSagittal());
    metrics.put("AngleT", angleTransverse());
    metrics.put("AngleC", angleCoronal());
    metrics.put("CrossSectionArea", crossSectionalArea());

    if ( withFullPath)
        _writeFullPath( pnode);
}   // end write


void Path::_writeFullPath( PTree& pnode) const
{
    PTree& fullpath = pnode.add( "FullPath", "");
    for ( const Vec3f &v : pathVertices())
        r3d::addVertex( fullpath, v);
}   // end _writeFullPath


void Path::_calcAngles( const Mat3f &iR)
{
    const Vec3f& h0 = handle0();    // First point in path
    const Vec3f& h1 = handle1();    // Last point in path
    const Vec3f& hd = depthHandle();
    // Apply the inverse rotation to get back vectors in standard position
    const Vec3f dh0 = iR*(h0 - hd);
    const Vec3f dh1 = iR*(h1 - hd);
    _angle = _calcAngle( dh0, dh1); // Direct angle
    _angleS = _calcAngle( Vec3f(  0.0f, dh0[1], dh0[2]), Vec3f(  0.0f, dh1[1], dh1[2]));   // Sagittal
    _angleT = _calcAngle( Vec3f(dh0[0],   0.0f, dh0[2]), Vec3f(dh1[0],   0.0f, dh1[2]));   // Transverse
    _angleC = _calcAngle( Vec3f(dh0[0], dh0[1],   0.0f), Vec3f(dh1[0], dh1[1],   0.0f));   // Coronal
}   // end _calcAngles


void Path::read( const PTree& pnode)
{
    std::string nm;
    if ( pnode.count("Name") > 0)
        nm = pnode.get<std::string>( "Name");
    else if ( pnode.count("<xmlattr>.name") > 0)
        nm = pnode.get<std::string>( "<xmlattr>.name");
    setName( nm);

    Vec3f v0 = Vec3f::Zero();
    if ( pnode.count("v0") > 0)
        v0 = r3d::getVertex( pnode.get_child("v0"));
    else if ( pnode.count("V0") > 0)
        v0 = r3d::getVertex( pnode.get_child("V0"));
    setHandle0( v0);

    Vec3f v1 = Vec3f::Zero();
    if ( pnode.count("v1") > 0)
        v1 = r3d::getVertex( pnode.get_child("v1"));
    else if ( pnode.count("V1") > 0)
        v1 = r3d::getVertex( pnode.get_child("V1"));
    setHandle1( v1);

    Vec3f vd = (v0 + v1)/2;
    if ( pnode.count("VD") > 0)
        vd = r3d::getVertex( pnode.get_child("VD"));
    setDepthHandle( vd);

    Vec3f u(0,0,1);
    if ( pnode.count("Orient") > 0)
        u = r3d::getVertex( pnode.get_child("Orient"));
    setOrientation( u);

    if ( pnode.count("Metrics") > 0)
    {
        const PTree &mnode = pnode.get_child("Metrics");
        if ( mnode.count("DirectDistance") > 0)
            _elen = mnode.get<float>("DirectDistance");
        if ( mnode.count("SurfaceDistance") > 0)
            _slen = mnode.get<float>("SurfaceDistance");
        if ( mnode.count("Depth") > 0)
            _depth = mnode.get<float>("Depth");
        if ( mnode.count("AngleAtDepth") > 0)   // Older versions
            _angle = mnode.get<float>("AngleAtDepth");
        if ( mnode.count("Angle") > 0)          // New version (overwrites)
            _angle = mnode.get<float>("Angle");
        if ( mnode.count("AngleS") > 0)
            _angleS = mnode.get<float>("AngleS");
        if ( mnode.count("AngleT") > 0)
            _angleT = mnode.get<float>("AngleT");
        if ( mnode.count("AngleC") > 0)
            _angleC = mnode.get<float>("AngleC");
        if ( mnode.count("CrossSectionArea") > 0)
            _area = mnode.get<float>("CrossSectionArea");
    }   // end if
}   // end read
