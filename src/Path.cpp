/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <ObjModelGeodesicPathFinder.h>
#include <ObjModelSurfacePointFinder.h>
#include <Orientation.h>    // RFeatures (for putVertex and getVertex)
#include <algorithm>
using FaceTools::Path;


Path::Path() : id(-1), elen(0), psum(0)
{
}   // end ctor


Path::Path( int i, const cv::Vec3f& v0) : id(i), elen(0), psum(0)
{
    vtxs.resize(2);
    vtxs[0] = vtxs[1] = v0;
    name = "";
}   // end ctor


void Path::recalculate( RFeatures::ObjModelKDTree::Ptr kdt)
{
    if ( vtxs.empty())
        return;

    cv::Vec3f v0 = vtxs[0];
    cv::Vec3f v1 = vtxs[vtxs.size()-1];

    // Set the path endpoints to be coincident with the model surface.
    int notused;
    cv::Vec3f vs0, vs1;
    const RFeatures::ObjModelSurfacePointFinder spfinder( kdt->model());
    int vidx0 = kdt->find(v0);
    int vidx1 = kdt->find(v1);
    spfinder.find( v0, vidx0, notused, vs0);
    spfinder.find( v1, vidx1, notused, vs1);
    v0 = vs0;
    v1 = vs1;

    // Recalculate the surface path.
    vtxs.clear();
    RFeatures::ObjModelGeodesicPathFinder pfinder(kdt.get());
    pfinder.findGeodesic( v0, v1, vtxs);    // vtxs now contains path
    vtxs.push_back(v0);
    std::reverse( std::begin(vtxs), std::end(vtxs));
    vtxs.push_back(v1);
    /*
    vtxs.resize(2);
    vtxs[0] = v0;
    vtxs[1] = v1;
    */

    elen = (float)cv::norm( v1-v0);    // The l2-norm (straight line distance)
    psum = 0;   // Calculate the path sum over the discovered path.
    cv::Vec3f vp = *vtxs.begin();
    for ( const cv::Vec3f& v : vtxs)
    {
        psum += (float)cv::norm( v - vp);
        vp = v;
    }   // end for
}   // end calculate


PTree& FaceTools::operator<<( PTree& pathsNode, const Path& p)
{
    PTree& pnode = pathsNode.add("path","");
    pnode.put( "<xmlattr>.name", p.name);
    RFeatures::putNamedVertex( pnode, "v0", *p.vtxs.begin());
    RFeatures::putNamedVertex( pnode, "v1", *p.vtxs.rbegin());
    PTree& metrics = pnode.add( "metrics", "");
    metrics.put("elen", p.elen);
    metrics.put("psum", p.psum);
    return pathsNode;
}   // end operator<<


const PTree& FaceTools::operator>>( const PTree& pnode, Path& p)
{
    p.name = pnode.get<std::string>( "<xmlattr>.name");
    p.vtxs.resize(2);
    p.vtxs[0] = RFeatures::getVertex( pnode.get_child("v0"));
    p.vtxs[1] = RFeatures::getVertex( pnode.get_child("v1"));
    // elen and plen need calculate via a call to recalculate
    return pnode;
}   // end operator>>
