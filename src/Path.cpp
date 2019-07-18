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

/*
namespace {

    // Set the vertex indices as those on the start and finish polygons (sT and fT)
    // that give a line segment that is most parallel to line segment v0-->v1.
    cv::Vec3d u;
    cv::normalize( v1-v0, u);
    v0i = findMostParallelVertex( u, model, v1, sT);
    v1i = findMostParallelVertex( u, model, v0, fT);

int findMostParallelVertex( const cv::Vec3d& u, const RFeatures::ObjModel* model, const cv::Vec3f& sv, int f)
{
    cv::Vec3d v;
    double m;
    int bi = 0;
    double d = 0;
    const int* vidxs = model->fvidxs(f);
    for ( int i = 0; i < 3; ++i)
    {
        cv::normalize( model->vtx(vidxs[i]) - sv, v);
        m = fabs(v.dot(u));
        if ( m > d)
        {
            d = m;
            bi = i;
        }   // end if
    }   // end for
    return vidxs[bi];
}   // end findMostParallelVertex

}   // end namespace
*/


bool Path::recalculate( const FM* fm)
{
    if ( vtxs.empty())
        return false;

    cv::Vec3f v0 = vtxs.front();
    cv::Vec3f v1 = vtxs.back();

    elen = cv::norm(v1-v0);    // The l2-norm (straight line distance)
    vtxs.clear();
    psum = 0;
    if ( findPath( fm, v0, v1, vtxs))
    {
        const cv::Vec3f* vp = &vtxs.front();
        for ( const cv::Vec3f& v : vtxs)
        {
            psum += cv::norm( v - *vp);
            vp = &v;
        }   // end for
    }   // end if

    return true;
}   // end calculate


PTree& FaceTools::operator<<( PTree& pathsNode, const Path& p)
{
    PTree& pnode = pathsNode.add("path","");
    pnode.put( "<xmlattr>.name", p.name);
    RFeatures::putNamedVertex( pnode, "v0", p.vtxs.front());
    RFeatures::putNamedVertex( pnode, "v1", p.vtxs.back());
    PTree& metrics = pnode.add( "metrics", "");
    metrics.put("elen", p.elen);
    metrics.put("psum", p.psum);
    return pathsNode;
}   // end operator<<


const PTree& FaceTools::operator>>( const PTree& pnode, Path& p)
{
    p.name = pnode.get<std::string>( "<xmlattr>.name");
    p.vtxs.resize(2);
    p.vtxs.front() = RFeatures::getVertex( pnode.get_child("v0"));
    p.vtxs.back()  = RFeatures::getVertex( pnode.get_child("v1"));
    // elen and plen need calculate via a call to recalculate
    return pnode;
}   // end operator>>

