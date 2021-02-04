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

#include <PathSet.h>
#include <cassert>
using FaceTools::PathSet;
using FaceTools::Path;
using FaceTools::FM;
using FaceTools::Vec3f;
using FaceTools::Mat4f;


PathSet::PathSet() : _sid(0) {}

PathSet::~PathSet(){}


void PathSet::update( const FM* fm)
{
    for ( auto& p : _paths)
    {
        p.second.update( fm);
        p.second.updateMeasures();
    }   // end for
}   // end update


int PathSet::_setPath( const Path& path)
{
    _paths[path.id()] = path;
    _ids.insert(path.id());
    return path.id();
}   // end _setPath


int PathSet::addPath( const Vec3f& v) { return _setPath( Path( _sid++, v));}   // end addPath


bool PathSet::removePath( int id)
{
    if ( _ids.count(id) == 0)
        return false;
    _paths.erase(id);
    _ids.erase(id);
    return true;
}   // end removePath


void PathSet::reset()
{
    _paths.clear();
    _ids.clear();
    _sid = 0;
}   // end reset


bool PathSet::renamePath( int id, const std::string& nm)
{
    if ( _ids.count(id) == 0)
        return false;
    _paths.at(id).setName(nm);
    return true;
}   // end renamePath


Path& PathSet::path( int pid)
{
    assert( has(pid));
    return _paths.at(pid);
}   // end path


const Path& PathSet::path( int pid) const
{
    assert( has(pid));
    return _paths.at(pid);
}   // end path


QString PathSet::name( int pid) const { return QString::fromStdString( path(pid).name());}


void PathSet::transform( const Mat4f& T)
{
    for ( auto& p : _paths)
    {
        p.second.transform(T);
        p.second.updateMeasures();
    }   // end for
}   // end transform


void PathSet::write( PTree& pathsNode, bool withExtras) const
{
    for ( const auto& p : _paths)
        p.second.write( pathsNode, withExtras);
}   // end write


bool PathSet::read( const PTree& pathsNode)
{
    for ( const PTree::value_type& lvt : pathsNode)
    {
        Path path;
        path.read( lvt.second);
        path.setId( _sid++);
        _setPath(path);
    }   // end for
    return true;
}   // end read

