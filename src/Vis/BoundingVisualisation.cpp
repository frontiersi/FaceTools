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

#include <Vis/BoundingVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Vis::FV;


void BoundingVisualisation::refresh( FV* fv)
{
    static const Vec3f COL( 0.0f, 0.2f, 0.9f);  // Default colour (blue)
    const FM *fm = fv->data();
    BoundingView &bv = _views[fv];
    bv.setColour( COL[0], COL[1], COL[2], 0.32f);
    bv.setLineStipplingEnabled( !fm->isAligned());
    bv.setLineWidth( 2.0f);
    bv.update( fm->bounds()[0]->cornersAs6f());
}   // end refresh


void BoundingVisualisation::setVisible( FV* fv, bool v)
{
    _views[fv].setVisible( v, fv->viewer());
}   // end setVisible


bool BoundingVisualisation::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).isVisible();
}   // end isVisible


void BoundingVisualisation::syncTransform( const FV* fv)
{
    _views[fv].pokeTransform( fv->transformMatrix());
}   // end syncTransform


void BoundingVisualisation::purge( const FV *fv)
{
    if (_views.count(fv) > 0)
        _views.erase(fv);
}   // end purge
