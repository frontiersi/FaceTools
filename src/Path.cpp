/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <ObjModelTools.h>
#include <Orientation.h>    // RFeatures (putVertex and getVertex)
#include <FaceTools.h>
#include <FaceModel.h>
#include <algorithm>
using FaceTools::Path;
using FaceTools::FM;


Path::Path() : id(-1), elen(0), psum(0) {}


Path::Path( int i, const cv::Vec3f& v0) : id(i), elen(0), psum(0)
{
    vtxs.resize(2);
    vtxs.front() = v0;
    vtxs.back() = v0;
    name = "";
}   // end ctor


bool Path::recalculate( const FM* fm)
{
    if ( vtxs.empty())
        return false;

    cv::Vec3f v0 = vtxs.front();
    cv::Vec3f v1 = vtxs.back();

    elen = cv::norm(v1-v0);    // The l2-norm (straight line distance)
    psum = 0;

    vtxs.clear();
    //if ( findPath( fm, v0, v1, vtxs)) // v4.1.0 of Cliniface
    //if ( findStraightPath( fm, v0, v1, cv::Vec3f(0,0,1), vtxs))
    if ( findCurveFollowingPath( fm, v0, v1, vtxs))
    {
        assert( !vtxs.empty());
        const cv::Vec3f* vp = &vtxs.front();
        for ( const cv::Vec3f& v : vtxs)
        {
            psum += cv::norm( v - *vp);
            vp = &v;
        }   // end for
    }   // end if
    else
    {
        vtxs.push_back(v0);
        vtxs.push_back(v1);
    }   // end else

    return true;
}   // end recalculate


PTree& FaceTools::operator<<( PTree& pathsNode, const Path& p)
{
    PTree& pnode = pathsNode.add("Path","");
    pnode.put( "Name", p.name);
    RFeatures::putNamedVertex( pnode, "V0", p.vtxs.front());
    RFeatures::putNamedVertex( pnode, "V1", p.vtxs.back());
    PTree& metrics = pnode.add( "Metrics", "");
    metrics.put("Elen", p.elen);
    metrics.put("Psum", p.psum);
    return pathsNode;
}   // end operator<<


const PTree& FaceTools::operator>>( const PTree& pnode, Path& p)
{
    p.name = "";
    if ( pnode.count("Name") > 0)
        p.name = pnode.get<std::string>( "Name");
    else if ( pnode.count("<xmlattr>.name") > 0)
        p.name = pnode.get<std::string>( "<xmlattr>.name");

    p.vtxs.resize(2);
    p.vtxs.front() = p.vtxs.back() = cv::Vec3f(0,0,0);
    if ( pnode.count("v0") > 0)
        p.vtxs.front() = RFeatures::getVertex( pnode.get_child("v0"));
    else if ( pnode.count("V0") > 0)
        p.vtxs.front() = RFeatures::getVertex( pnode.get_child("V0"));
    if ( pnode.count("v1") > 0)
        p.vtxs.back()  = RFeatures::getVertex( pnode.get_child("v1"));
    else if ( pnode.count("V1") > 0)
        p.vtxs.back()  = RFeatures::getVertex( pnode.get_child("V1"));

    // elen and plen need calculate via a call to recalculate
    return pnode;
}   // end operator>>

