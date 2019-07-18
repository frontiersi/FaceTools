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

#include <PathSetVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
#include <vtkProperty.h>
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::PathSetView;
using FaceTools::Vis::PathView;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::Path;
using FaceTools::Action::Event;


PathSetVisualisation::~PathSetVisualisation()
{
    while (!_views.empty())
        purge( const_cast<FV*>(_views.begin()->first), Event::NONE);
}   // end dtor


bool PathSetVisualisation::isAvailable( const FM* fm) const
{
    assert(fm);
    return !fm->paths().empty();
}   // end isAvailable


bool PathSetVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{
    return pathHandle( fv, prop) != nullptr;
}   // end belongs


void PathSetVisualisation::apply( FV* fv, const QPoint*)
{
    if ( !hasView(fv))
        _views[fv] = new PathSetView( fv->data()->paths());
    FMV* viewer = fv->viewer();
    _views.at(fv)->setVisible( true, viewer);
}   // end apply


bool PathSetVisualisation::purge( FV* fv, Event)
{
    if ( hasView(fv))
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
    return true;
}   // end purge


void PathSetVisualisation::setVisible( FV* fv, bool v)
{
    if ( hasView(fv))
    {
        FMV* viewer = fv->viewer();
        _views.at(fv)->setVisible( v, viewer);
    }   // end if
}   // end setVisible


bool PathSetVisualisation::isVisible( const FV *fv) const
{
    bool vis = false;
    if ( hasView(fv))
        vis = _views.at(fv)->isVisible();
    return vis;
}   // end isVisible


// public
void PathSetVisualisation::addPath( const FM* fm, int pathId)
{
    for ( FV* fv : fm->fvs())
    {
        if ( !hasView(fv))
            apply(fv,nullptr);
        else
            _views.at(fv)->updatePath(pathId);
        _views.at(fv)->showPath( true, pathId);
    }   // end for
}   // end addPath


void PathSetVisualisation::updatePath( const FM* fm, int pathId)
{
    for ( FV* fv : fm->fvs())
        if ( hasView(fv))
            _views.at(fv)->updatePath( pathId);
}   // end updatePath


void PathSetVisualisation::refresh( const FM* fm)
{
    for ( FV* fv : fm->fvs())
        if ( hasView(fv))
            _views.at(fv)->refresh();
}   // end refresh


void PathSetVisualisation::setText( const FM* fm, int pid, int xpos, int ypos)
{
    for ( FV* fv : fm->fvs())
        if ( hasView(fv))
            _views.at(fv)->setText( pid, xpos, ypos);
}   // end setText


void PathSetVisualisation::showText( const FM* fm)
{
    for ( auto& p : _views)
        p.second->setTextVisible(false);

    if ( fm)
    {
        for ( FV* fv : fm->fvs())
            if ( hasView(fv))
                _views.at(fv)->setTextVisible(true);
    }   // end if
}   // end showText


PathView::Handle* PathSetVisualisation::pathHandle( const FV* fv, const vtkProp* prop) const
{
    PathView::Handle *h = nullptr;
    if ( hasView(fv))
        h = _views.at(fv)->handle( prop);
    return h;
}   // end pathHandle


PathView::Handle* PathSetVisualisation::pathHandle0( const FV* fv, int pid) const
{
    PathView::Handle* h = nullptr;
    if ( hasView(fv))
    {
        PathView* pv = _views.at(fv)->pathView(pid);
        if ( pv)
            h = pv->handle0();
    }   // end if
    return h;
}   // end pathHandle0


PathView::Handle* PathSetVisualisation::pathHandle1( const FV* fv, int pid) const
{
    PathView::Handle* h = nullptr;
    if ( hasView(fv))
    {
        PathView* pv = _views.at(fv)->pathView(pid);
        if ( pv)
            h = pv->handle1();
    }   // end if
    return h;
}   // end pathHandle1


void PathSetVisualisation::syncActorsToData(const FV *fv, const cv::Matx44d &d)
{
    if ( hasView(fv))
        _views.at(fv)->refresh( d);
}   // end syncActorsToData


// private
bool PathSetVisualisation::hasView( const FV* fv) const { return _views.count(fv) > 0;}
