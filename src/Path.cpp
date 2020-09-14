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

#include <Path.h>
#include <r3d/Orientation.h>
#include <FaceTools.h>
#include <FaceModel.h>
using FaceTools::Path;
using FaceTools::FM;
using FaceTools::Vec3f;
using FaceTools::Mat4f;


// static definition
Path::PathType Path::s_pathType( Path::ORIENTED_CURVE);

void Path::setPathType( Path::PathType pt) { s_pathType = pt;}


Path::Path()
    : _id(-1), _name(""), _validPath(false),
    _elen(0), _slen(0), _area(0), _depth(0), _angle(0), _dhan(0.5f)
{
    _vtxs.resize(2);
}   // end ctor


Path::Path( int i, const Vec3f& v)
    : _id(i), _name(""), _validPath(false),
    _elen(0), _slen(0), _area(0), _depth(0), _angle(0), _dhan(0.5f)
{
    _vtxs.resize(2);
    _vtxs.front() = _vtxs.back() = v;
}   // end ctor


bool Path::update( const FM* fm)
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
                _validPath = findCurveFollowingPath( fm->kdtree(), v0, v1, _vtxs);   // Experimental
                break;
            case STRAIGHT_CURVE:
                _validPath = findStraightPath( fm->kdtree(), v0, v1, _vtxs);    // Default
                break;
            case ORIENTED_CURVE:
                assert( u != Vec3f::Zero());
                _validPath = findOrientedPath( fm->kdtree(), v0, v1, u, _vtxs);
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
}   // end update


void Path::updateMeasures()
{
    const Vec3f& h0 = handle0();    // First point in path
    const Vec3f& h1 = handle1();    // Last point in path
    const Vec3f hd = depthHandle();

    _slen = 0.0f;
    _area = 0.0f;
    _depth = 0.0f;

    Vec3f u = h1 - h0;  // Could be zero vector
    _elen = u.norm();   // May be zero
    if ( _elen > 0.0f)
        u = u / _elen;  // Euclidean direction between end points

    // Find two vectors va and vb where va = a-h0, and |va| < dh, and vb = b-h0, and |vb| >= dh,
    // and a and b are consecutive points in the surface path.
    const float dh = (hd - h0).norm(); // Magnitude to the depth handle from handle 0

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
        if ( d < dh && !b)
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

    // dh is in [da,db] and vertices a and b are the endpoints of a single path segment.
    // Get the exact point along this segment where depth should be measured from.
    _dmax = *a;
    if ( a != b)
        _dmax += (*b-*a) * ((dh - da) / (db - da));
    _depth = (hd - _dmax).norm();
    _angle = 180.0f;

    if ( _depth > 0.0f)
    {
        static const float DEG_CONST = 180.0f/EIGEN_PI;
        const Vec3f dh0 = h0 - _dmax;
        const Vec3f dh1 = h1 - _dmax;
        const float denom = dh0.norm() * dh1.norm();
        assert( denom > 0.0f);
        _angle = acosf( dh0.dot(dh1)/denom) * DEG_CONST;
        if ( std::isnan(_angle))
            _angle = 180.0f;
    }   // end if
}   // end updateMeasures


void Path::transform( const Mat4f &t)
{
    for ( Vec3f &v : _vtxs)
        v = r3d::transform( t, v);
    _dmax = r3d::transform( t, _dmax);
    _orient = t.block<3,3>(0,0) * _orient;
    _orient.normalize();
}   // end transform


void Path::setHandle0( const Vec3f& v) { _vtxs.front() = v;}
void Path::setHandle1( const Vec3f& v) { _vtxs.back() = v;}
void Path::setDepthHandle( float v) { _dhan = std::max( 0.0f, std::min( v, 1.0f));}
Vec3f Path::depthHandle() const { return handle0() + _dhan * (handle1() - handle0());}
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
        default:
            setOrientation(v);
    }   // end switch
}   // end setHandle


const Vec3f& Path::handle( int h) const
{
    const Vec3f *v = nullptr;
    switch (h)
    {
        case 0:
            v = &handle0();
            break;
        case 1:
            v = &handle1();
            break;
        default:
            v = &orientation();
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
    metrics.put("AngleAtDepth", angleAtDepth());
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
    setDepthHandle( (vd - v0).norm() / (v1 - v0).norm());

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
        if ( mnode.count("AngleAtDepth") > 0)
            _angle = mnode.get<float>("AngleAtDepth");
        if ( mnode.count("CrossSectionArea") > 0)
            _area = mnode.get<float>("CrossSectionArea");
    }   // end if
}   // end read
