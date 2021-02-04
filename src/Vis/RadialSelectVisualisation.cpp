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

#include <Vis/RadialSelectVisualisation.h>
#include <Interactor/RadialSelectHandler.h>
#include <FaceModelViewer.h>
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::RadialSelectView;
using FaceTools::Vis::FV;


RadialSelectVisualisation::RadialSelectVisualisation() : _handler(nullptr) {}


bool RadialSelectVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).belongs(prop);
}   // end belongs


bool RadialSelectVisualisation::isAvailable( const FV *fv) const { return true;}


void RadialSelectVisualisation::setVisible( FV* fv, bool v)
{
    assert( _views.count(fv) > 0);
    _views.at(fv).setVisible( v, fv->viewer());
}   // end setVisible


bool RadialSelectVisualisation::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).isVisible();
}   // end isVisible


void RadialSelectVisualisation::syncTransform( const FV *fv)
{
    assert( _views.count(fv) > 0);
    _views.at(fv).pokeTransform( fv->transformMatrix());
}   // end syncTransform


void RadialSelectVisualisation::purge( const FV* fv) { _views.erase(fv);}


void RadialSelectVisualisation::purgeAll()
{
    // Need to copy out the FaceViews as non-const first so the purge calls
    // don't erase items from the container we're iterating over. This is
    // some crappy design frankly - need to think of a better way of doing
    // this more generally.
    std::vector<FV*> fvs;
    fvs.reserve( _views.size());
    for ( const auto &p : _views)
        fvs.push_back(const_cast<FV*>(p.first));
    for ( FV *fv : fvs)
        fv->purge(this);
}   // end purgeAll


void RadialSelectVisualisation::refresh( FV *fv)
{
    if ( _views.count(fv) == 0)
        update( fv->data());
    setHighlighted( fv, false);
}   // end refresh


void RadialSelectVisualisation::update( const FM *fm)
{
    // Get the untransformed centre
    const Vec3f c = r3d::transform( fm->inverseTransformMatrix(), _handler->centre());
    // Get the list of vertices
    const std::list<int> &vidxs = _handler->boundaryVertices();
    // Update the RadialSelectView of each FaceView and render immediately
    for ( const FV *fv : fm->fvs())
        _views[fv].update( c, fm->mesh(), vidxs);
}   // end update


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
