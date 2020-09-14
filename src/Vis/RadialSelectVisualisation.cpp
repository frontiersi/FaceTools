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

#include <Vis/RadialSelectVisualisation.h>
#include <Vis/FaceView.h>
#include <Interactor/RadialSelectHandler.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::RadialSelectView;
using FaceTools::Vis::FV;
using FaceTools::FM;


RadialSelectVisualisation::RadialSelectVisualisation() : _handler(nullptr) {}


RadialSelectVisualisation::~RadialSelectVisualisation()
{
    while (!_views.empty())
        purge( _views.begin()->first);
}   // end dtor


bool RadialSelectVisualisation::isAvailable( const FV *fv, const QPoint*) const
{
    return true;
}   // end isAvailable


bool RadialSelectVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).belongs(prop);
}   // end belongs


void RadialSelectVisualisation::apply( const FV* fv, const QPoint*)
{
    _views[fv]; // Just ensure present
    setHighlighted( fv, false);
}   // end apply


void RadialSelectVisualisation::purge( const FV* fv)
{
    if ( _views.count(fv) > 0)
        _views.erase(fv);
}   // end purge


void RadialSelectVisualisation::setVisible( FV* fv, bool v)
{
    if ( _views.count(fv) > 0)
        _views.at(fv).setVisible( v, fv->viewer());
}   // end setVisible


bool RadialSelectVisualisation::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).isVisible();
}   // end isVisible


void RadialSelectVisualisation::syncWithViewTransform( const FV *fv)
{
    if ( _views.count(fv) > 0)
        _views.at(fv).pokeTransform( fv->transformMatrix());
}   // end syncWithViewTransform


void RadialSelectVisualisation::refresh( const FV *fv)
{
    assert(_handler);
    assert(fv);
    const FM *fm = fv->data();
    const r3d::Mesh &mesh = fm->mesh();
    // Get the untransformed centre
    const Vec3f upos = r3d::transform( fm->inverseTransformMatrix(), _handler->centre());
    // Get the untransformed vertices
    const std::list<int> &vidxs = _handler->boundaryVertices();
    std::vector<const Vec3f*> vvec( vidxs.size());
    int i = 0;
    for ( int vidx : vidxs)
        vvec[i++] = &mesh.uvtx(vidx);

    for ( const FV *f : fm->fvs())
        if ( _views.count(f) > 0)
            _views.at(f).update( upos, vvec);
}   // end refresh


void RadialSelectVisualisation::setHighlighted( const FV* fv, bool hval)
{
    if ( _views.count(fv) > 0)
    {
        static const Vec3f hcol( 0.9f, 0.1f, 0.0f);
        static const Vec3f ncol( 0.2f, 0.7f, 0.0f);
        const Vec3f *col = hval ? &hcol : &ncol;
        _views.at(fv).setColour( (*col)[0], (*col)[1], (*col)[2], 0.6);
    }   // end if
}   // end setHighlighted
