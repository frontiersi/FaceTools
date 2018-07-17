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

#include <PathSetView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::PathSetView;
using FaceTools::Vis::PathView;
using FaceTools::ModelViewer;
using FaceTools::PathSet;
using FaceTools::Path;


// public
PathSetView::PathSetView( const PathSet::Ptr paths) : _paths(paths), _viewer(nullptr)
{
    for ( int id : paths->ids())
        addPath(id);
}   // end ctor


// public
PathSetView::~PathSetView()
{
    std::for_each( std::begin(_pviews), std::end(_pviews), [](auto p){ delete p.second;});
}   // end dtor


// public
bool PathSetView::isVisible() const { return !_visible.empty();}


// public
void PathSetView::setVisible( bool enable, ModelViewer *viewer)
{
    while ( isVisible())
        showPath( false, *_visible.begin());
    _viewer = viewer;
    if ( enable && _viewer)
        std::for_each( std::begin(_pviews), std::end(_pviews), [this](auto p){ this->showPath(true, p.first);});
}   // end setVisible


// public
void PathSetView::showPath( bool enable, int id)
{
    assert( _pviews.count(id) > 0);
    _pviews.at(id)->setVisible( enable, _viewer);
    _visible.erase(id);
    if ( isPathVisible(id))
        _visible.insert(id);
}   // end showPath


// public
bool PathSetView::isPathVisible( int id) const { return _pviews.at(id)->isVisible();}
const std::unordered_set<int>& PathSetView::visible() const { return _visible;}


// public
PathView::Handle* PathSetView::addPath( int id)
{
    const Path* path = _paths->path(id);
    assert(path);
    PathView* pv = new PathView( *path);
    _pviews[id] = pv;
    // Map handle props to the handles themselves for fast lookup.
    _handles[pv->handle0()->prop()] = pv->handle0();
    _handles[pv->handle1()->prop()] = pv->handle1();
    return pv->handle0();
}   // end addPath


// public
PathView::Handle* PathSetView::handle( const vtkProp* prop) const
{
    PathView::Handle *h = nullptr;
    if ( _handles.count(prop) > 0)
        h = _handles.at(prop);
    return h;
}   // end handle


// public
PathView* PathSetView::pathView( int id) const
{
    PathView* pv = nullptr;
    if ( _pviews.count(id) > 0)
        pv = _pviews.at(id);
    return pv;
}   // end pathView


// public
void PathSetView::updatePath( int id)
{
    if ( _paths->has(id))
    {
        assert(_pviews.count(id) > 0);
        _pviews.at(id)->update();
    }   // end if
    else if (_pviews.count(id) > 0)
    {   // Delete the path if no longer in the dataset and was shown previously
        showPath(false, id);
        PathView* pv = _pviews.at(id);
        _handles.erase(pv->handle0()->prop());
        _handles.erase(pv->handle1()->prop());
        _pviews.erase(id);
        delete pv;
    }   // end else if
}   // end updatePath


// public
void PathSetView::updatePaths()
{
    std::for_each( std::begin(_pviews), std::end(_pviews), [this](auto p){ this->updatePath(p.first);});
}   // end updatePaths


// public
void PathSetView::pokeTransform( const vtkMatrix4x4* m)
{
    std::for_each( std::begin(_pviews), std::end(_pviews), [=](auto p){ p.second->pokeTransform(m);});
}   // end pokeTransform


// public
void PathSetView::fixTransform()
{
    std::for_each( std::begin(_pviews), std::end(_pviews), [=](auto p){ p.second->fixTransform();});
}   // end fixTransform
