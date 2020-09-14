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

#include <Vis/PathSetVisualisation.h>
#include <Vis/FaceView.h>
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


PathSetVisualisation::PathSetVisualisation() : _maxOpacity(1.0f) {}


PathSetVisualisation::~PathSetVisualisation()
{
    while (!_views.empty())
        purge( _views.begin()->first);
}   // end dtor


bool PathSetVisualisation::isAvailable( const FV *fv, const QPoint*) const
{
    return !fv->data()->currentPaths().empty();
}   // end isAvailable


bool PathSetVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{
    return pathHandle( fv, prop) != nullptr;
}   // end belongs


void PathSetVisualisation::setMaxAllowedOpacity( float v)
{
    _maxOpacity = std::min<float>( 1.0f, std::max<float>( minAllowedOpacity(), v));
}   // end setMaxAllowedOpacity


void PathSetVisualisation::apply( const FV* fv, const QPoint*)
{
    assert(fv);
    if ( !_hasView(fv))
    {
        _views[fv] = new PathSetView;
        PathSet pset = fv->data()->currentPaths();  // Copy out
        pset.transform( fv->data()->inverseTransformMatrix());  // Transform paths for visualising
        _views.at(fv)->sync( pset);
    }   // end if
}   // end apply


void PathSetVisualisation::purge( const FV* fv)
{
    if ( _hasView(fv))
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
}   // end purge


void PathSetVisualisation::setVisible( FV* fv, bool v)
{
    if ( _hasView(fv))
        _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool PathSetVisualisation::isVisible( const FV* fv) const
{
    return _hasView(fv) && _views.at(fv)->isVisible();
}   // end isVisible


void PathSetVisualisation::addPath( const FM* fm, int pathId)
{
    Path path = fm->currentPaths().path(pathId);  // Copy out
    path.transform( fm->inverseTransformMatrix());  // Transform for visualising
    for ( FV* fv : fm->fvs())
    {
        if ( !_hasView(fv))
            apply( fv, nullptr);
        else
            _views[fv]->addPath( path);
        _views.at(fv)->pokeTransform( r3dvis::toVTK( fm->transformMatrix()));
    }   // end for
}   // end addPath


void PathSetVisualisation::updatePath( const FM* fm, int pathId)
{
    Path path = fm->currentPaths().path(pathId);
    path.transform( fm->inverseTransformMatrix());
    for ( FV* fv : fm->fvs())
    {
        if ( _hasView(fv))
        {
            _views.at(fv)->updatePath( path);
            _views.at(fv)->pokeTransform( r3dvis::toVTK( fm->transformMatrix()));
        }   // end if
    }   // end for
}   // end updatePath


void PathSetVisualisation::erasePath( const FM* fm, int pathId)
{
    for ( FV* fv : fm->fvs())
        if ( _hasView(fv))
            _views.at(fv)->erasePath(pathId);
}   // end erasePath


void PathSetVisualisation::syncPaths( const FM* fm)
{
    if ( !fm)
        return;

    PathSet pset = fm->currentPaths();
    pset.transform( fm->inverseTransformMatrix());
    for ( FV* fv : fm->fvs())
    {
        if ( _hasView(fv))
        {
            _views.at(fv)->sync( pset);
            _views.at(fv)->pokeTransform( r3dvis::toVTK( fm->transformMatrix()));
        }   // end if
    }   // end for
}   // end syncPaths


void PathSetVisualisation::updateCaption( const FM* fm, int pid, int xpos, int ypos)
{
    const Mat3f iR = fm->inverseTransformMatrix().block<3,3>(0,0);
    const Path& path = fm->currentPaths().path(pid);
    for ( FV* fv : fm->fvs())
        if ( _hasView(fv))
            _views.at(fv)->setCaption( path, xpos, ypos, iR);
}   // end updateCaption


void PathSetVisualisation::showCaption( const FV *fv)
{
    for ( auto& p : _views)
        p.second->setCaptionVisible(false);
    if ( _hasView(fv))
        _views.at(fv)->setCaptionVisible(true);
}   // end showCaption


PathView::Handle* PathSetVisualisation::pathHandle( const FV* fv, const vtkProp* prop) const
{
    return _hasView(fv) ? _views.at(fv)->handle( prop) : nullptr;
}   // end pathHandle


PathView::Handle* PathSetVisualisation::pathHandle0( const FV* fv, int pid) const
{
    PathView::Handle* h = nullptr;
    if ( _hasView(fv))
    {
        PathView* pv = _views.at(fv)->pathView(pid);
        if ( pv)
            h = pv->handle0();
    }   // end if
    return h;
}   // end pathHandle0


void PathSetVisualisation::syncWithViewTransform( const FV* fv)
{
    if ( _hasView(fv))
        _views.at(fv)->pokeTransform( fv->transformMatrix());
}   // end syncWithViewTransform


void PathSetVisualisation::refresh( const FV *fv)
{
    if ( _hasView(fv))
        _views.at(fv)->updateTextColours();
}   // end refresh


bool PathSetVisualisation::_hasView( const FV* fv) const { return fv && _views.count(fv) > 0;}
