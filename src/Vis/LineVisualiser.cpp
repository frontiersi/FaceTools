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

#include <Vis/LineVisualiser.h>
#include <FaceModelViewer.h>
using FaceTools::Vis::LineVisualiser;
using FaceTools::Vis::FV;


bool LineVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _views.count(fv) > 0)
        for ( const LineView &lv : _views.at(fv))
            if ( lv.belongs(p))
                return true;
    return false;
}   // end belongs


bool LineVisualiser::isVisible( const FV *fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).at(0).isVisible();
}   // end isVisible


void LineVisualiser::syncTransform( const FV *fv)
{
    assert( _views.count(fv) > 0);
    if ( _views.count(fv) > 0)
        for ( LineView &lv : _views.at(fv))
            lv.pokeTransform( fv->transformMatrix());
}   // end syncTransform


void LineVisualiser::purge( const FV *fv) { _views.erase(fv);}


void LineVisualiser::setVisible( FV* fv, bool v)
{
    if ( _views.count(fv) > 0)
        for ( LineView &lv : _views.at(fv))
            lv.setVisible( v, fv->viewer());
}   // end setVisible


void LineVisualiser::setHighlighted( bool v)
{
    const double lw = v ? 5.0 : 1.0;
    for ( auto &p : _views)
        for ( LineView &lv : p.second)
            lv.setLineWidth( lw);
}   // end setHighlighted
