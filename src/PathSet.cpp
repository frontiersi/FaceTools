/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

// public
PathSet::Ptr PathSet::create()
{
    return Ptr( new PathSet, [](auto d){delete d;});
}   // end create


// private
PathSet::PathSet() : _sid(0) {}

// private
PathSet::~PathSet(){}


// public
void PathSet::recalculate( RFeatures::ObjModelKDTree::Ptr kdt)
{
    std::for_each( std::begin(_paths), std::end(_paths), [&](auto& p){ p.second.recalculate( kdt);});
}   // end calculate


// private
int PathSet::setPath( const Path& path)
{
    _paths[path.id] = path;
    _ids.insert(path.id);
    return path.id;
}   // end setPath


// public
int PathSet::addPath( const cv::Vec3f& v) { return setPath( Path( _sid++, v));}


// public
bool PathSet::removePath( int id)
{
    if ( _ids.count(id) == 0)
        return false;
    _paths.erase(id);
    _ids.erase(id);
    return true;
}   // end removePath


// public
Path* PathSet::path( int pid)
{
    if ( !has(pid))
        return nullptr;
    return &_paths.at(pid);
}   // end path


// public
const Path* PathSet::path( int pid) const
{
    if ( !has(pid))
        return nullptr;
    return &_paths.at(pid);
}   // end path


// public
void PathSet::transform( const cv::Matx44d& T)
{
    const RFeatures::Transformer mover(T);
    for ( auto& p : _paths)
    {
        std::vector<cv::Vec3f>& vtxs = p.second.vtxs;
        std::for_each(std::begin(vtxs), std::end(vtxs), [&](cv::Vec3f& v) { mover.transform(v); });  // Move in-place
    }   // end for
}   // end transform


PTree& FaceTools::operator<<( PTree& record, const PathSet& ps)
{
    PTree& pathsNode = record.put("paths","");
    std::for_each( std::begin(ps._paths), std::end(ps._paths), [&](const auto& p){ pathsNode << p.second;});
    return record;
}   // end operator<<


const PTree& FaceTools::operator>>( const PTree& record, PathSet& ps)
{
    if ( record.count("paths") == 0)
    {
        std::cerr << "[WARNING] FaceTools::operator>> (PathSet) : \"paths\" child not found in record - skipping!" << std::endl;
        return record;
    }   // end if

    const PTree& pathsNode = record.get_child("paths");
    for ( const PTree::value_type& lvt : pathsNode)
    {
        Path path;
        lvt.second >> path;
        path.id = ps._sid++;
        ps.setPath(path);
    }   // end for
    return record;
}   // end operator>>

