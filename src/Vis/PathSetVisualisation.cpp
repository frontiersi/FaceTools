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

#include <Vis/PathSetVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::PathView;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Path;


PathSetVisualisation::PathSetVisualisation() : _maxOpacity(1.0f) {}


bool PathSetVisualisation::isAvailable( const FV *fv) const
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


void PathSetVisualisation::refresh( FV *fv)
{
    const FM *fm = fv->data();
    PathSet pset = fm->currentPaths();  // Copy out
    pset.transform( fm->inverseTransformMatrix());  // Transform paths for visualising
    vtkSmartPointer<vtkMatrix4x4> tmat = r3dvis::toVTK( fm->transformMatrix());
    for ( FV *f : fm->fvs())
    {
        _views[f].sync(pset);
        _views.at(f).pokeTransform( tmat);
        _views.at(f).updateTextColours();
        _views.at(f).setVisible( true, f->viewer());
    }   // end for
}   // end refresh


void PathSetVisualisation::purge( const FV* fv)
{
    if ( _hasView(fv))
        _views.erase(fv);
}   // end purge


void PathSetVisualisation::setVisible( FV* fv, bool v)
{
    if ( _hasView(fv))
    {
        _views.at(fv).setVisible( v, fv->viewer());
        _views.at(fv).updateTextColours();
    }   // end if
}   // end setVisible


void PathSetVisualisation::setPickable( const FV* fv, bool v)
{
    if ( _hasView(fv))
        _views.at(fv).setPickable( v);
}   // end setPickable


bool PathSetVisualisation::isVisible( const FV* fv) const
{
    return _hasView(fv) && _views.at(fv).isVisible();
}   // end isVisible


void PathSetVisualisation::updatePath( const FM &fm, int pathId)
{
    Path path = fm.currentPaths().path(pathId);
    path.transform( fm.inverseTransformMatrix());
    vtkSmartPointer<vtkMatrix4x4> tmat = r3dvis::toVTK( fm.transformMatrix());
    for ( const FV* fv : fm.fvs())
    {
        assert( _hasView(fv));
        _views.at(fv).sync( path);
        _views.at(fv).pokeTransform( tmat);
    }   // end for
}   // end updatePath


void PathSetVisualisation::erasePath( const FM &fm, int pathId)
{
    for ( const FV* fv : fm.fvs())
    {
        assert( _hasView(fv));
        _views.at(fv).erasePath(pathId);
    }   // end for
}   // end erasePath


void PathSetVisualisation::showTemporaryPath( const FM &fm, const Path &inPath,
                                                int hid, const QString &hcap)
{
    Path path = inPath;
    path.transform( fm.inverseTransformMatrix());
    vtkSmartPointer<vtkMatrix4x4> tmat = r3dvis::toVTK( fm.transformMatrix());
    for ( FV *f : fm.fvs())
    {
        PathSetView &psv = _tviews[f];
        psv.sync(path);
        psv.pokeTransform( tmat);
        psv.pathView(path.id())->handle(hid)->setCaption(hcap);
        psv.pathView(path.id())->handle(hid)->showCaption(true);
        psv.setVisible( true, f->viewer());
        psv.setCaptionVisible( true);
        psv.updateTextColours();
    }   // end for
}   // end showTemporaryPath


void PathSetVisualisation::clearTemporaryPath()
{
    for ( auto &p : _tviews)
        p.second.setVisible( false, p.first->viewer());
    _tviews.clear();
}   // end clearTemporaryPath


void PathSetVisualisation::updateCaption( const FM &fm, const Path &path)
{
    static const int ypos = 10;
    const Mat3f iR = fm.inverseTransformMatrix().block<3,3>(0,0);
    for ( FV *fv : fm.fvs())
    {
        const int xpos = int(fv->viewer()->getWidth()) - 10;
        if ( _views.count(fv) > 0)
            _views.at(fv).setCaption( path, xpos, ypos, iR);
        else if ( _tviews.count(fv) > 0)
            _tviews.at(fv).setCaption( path, xpos, ypos, iR);
    }   // end for
}   // end updateCaption


void PathSetVisualisation::showCaption( const FV *fv)
{
    for ( auto& p : _views)
        p.second.setCaptionVisible(false);
    if ( _hasView(fv))
        _views.at(fv).setCaptionVisible(true);
}   // end showCaption


PathView::Handle* PathSetVisualisation::pathHandle( const FV* fv, const vtkProp* prop) const
{
    return _hasView(fv) ? _views.at(fv).handle( prop) : nullptr;
}   // end pathHandle


PathView::Handle* PathSetVisualisation::pathHandle0( const FV* fv, int pid) const
{
    PathView::Handle* h = nullptr;
    if ( _hasView(fv))
    {
        PathView* pv = _views.at(fv).pathView(pid);
        if ( pv)
            h = pv->handle0();
    }   // end if
    return h;
}   // end pathHandle0


void PathSetVisualisation::syncTransform( const FV* fv)
{
    if ( _hasView(fv))
        _views.at(fv).pokeTransform( fv->transformMatrix());
}   // end syncTransform


bool PathSetVisualisation::_hasView( const FV* fv) const { return fv && _views.count(fv) > 0;}
