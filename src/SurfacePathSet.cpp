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

#include <SurfacePathSet.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::SurfacePathSet;
using FaceTools::SurfacePath;
using FaceTools::FaceControl;


// public
SurfacePathSet::SurfacePathSet( const FaceControl* fc)
    : _fcont(fc)
{
    assert( fc->viewer());  // Must have a viewer!
}   // end ctor


// public
SurfacePathSet::~SurfacePathSet()
{
    while ( !_paths.empty())
        deletePath( *_paths.begin());
}   // end dtor


// public
SurfacePath::Handle* SurfacePathSet::path( const QPoint& p) const
{
    const vtkProp* prop = _fcont->viewer()->getPointedAt(p);
    if ( _handlesMap.count(prop) == 0) // Lookup prop from the set of handles of paths
        return NULL;
    return _handlesMap.at(prop);
}   // end path


// public
SurfacePath::Handle* SurfacePathSet::createPath( const QPoint& p)
{
    SurfacePath* path = new SurfacePath( _fcont, p);
    _paths.insert(path);
    _handlesMap[path->handle0()->actor()] = path->handle0();
    _handlesMap[path->handle1()->actor()] = path->handle1();
    return path->handle1();
}   // end createPath


// public
void SurfacePathSet::updatePath( SurfacePath::Handle* handle, const QPoint& p)
{
    assert( _paths.count(handle->host()) > 0);  // Illegal to update handle of path not owned by this PathSet!
    handle->set(p);
}   // end updatePath


// public
void SurfacePathSet::deletePath( SurfacePath* path)
{
    assert( _paths.count( path) > 0);  // Illegal to update handle of path not owned by this PathSet!
    _handlesMap.erase( path->handle0()->actor());
    _handlesMap.erase( path->handle1()->actor());
    _paths.erase( path);
    delete path;
}   // end deletePath
