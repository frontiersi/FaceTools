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

#include <PathSet.h>
#include <algorithm>
#include <Transformer.h>    // RFeatures
using FaceTools::PathSet;
using FaceTools::Path;
using PathPair = std::pair<int, Path>;

PathSet::Ptr PathSet::create()
{
    return Ptr( new PathSet, [](PathSet* d){delete d;});
}   // end create


// private
PathSet::PathSet() : _sid(0) {}

// private
PathSet::~PathSet(){}


void PathSet::recalculate( RFeatures::ObjModelKDTree::Ptr kdt)
{
    for ( auto& p : _paths)
        p.second.recalculate(kdt);
}   // end calculate


// private
int PathSet::setPath( const Path& path)
{
    _paths[path.id] = path;
    _ids.insert(path.id);
    return path.id;
}   // end setPath


int PathSet::addPath( const cv::Vec3f& v) { return setPath( Path( _sid++, v));}


bool PathSet::removePath( int id)
{
    if ( _ids.count(id) == 0)
        return false;
    _paths.erase(id);
    _ids.erase(id);
    return true;
}   // end removePath


Path* PathSet::path( int pid)
{
    if ( !has(pid))
        return nullptr;
    return &_paths.at(pid);
}   // end path


const Path* PathSet::path( int pid) const
{
    if ( !has(pid))
        return nullptr;
    return &_paths.at(pid);
}   // end path


void PathSet::transform( const cv::Matx44d& T)
{
    const RFeatures::Transformer mover(T);
    for ( auto& p : _paths)
    {
        std::vector<cv::Vec3f>& vtxs = p.second.vtxs;
        std::for_each(std::begin(vtxs), std::end(vtxs), [&](cv::Vec3f& v) { mover.transform(v); });  // Move in-place
    }   // end for
}   // end transform


void PathSet::write( PTree& pathsNode)
{
    std::for_each( std::begin(_paths), std::end(_paths), [&](const PathPair& p){ pathsNode << p.second;});
}   // end operator<<


bool PathSet::read( const PTree& pathsNode)
{
    for ( const PTree::value_type& lvt : pathsNode)
    {
        Path path;
        lvt.second >> path;
        path.id = _sid++;
        setPath(path);
    }   // end for
    return true;
}   // end operator>>

