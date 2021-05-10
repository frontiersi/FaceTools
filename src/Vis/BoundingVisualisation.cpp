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
    static const Vec3f ACOL( 0.0f, 0.1f, 0.9f);  // Aligned colour (blue)
    static const Vec3f UCOL( 0.7f, 0.0f, 0.8f);  // Unaligned colour (purple)
    const FM *fm = fv->data();
    const Vec3f &COL = fm->isAligned() ? ACOL : UCOL;
    BoundingView &bv = _views[fv];
    bv.update( fm->bounds()[0]->cornersAs6f(), fm->isAligned() ? 1 : 3);
    bv.setColour( COL[0], COL[1], COL[2], 0.32f);
    bv.setLineWidth( fm->isAligned() ? 1.0f : 3.0f);
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
