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

#include <Vis/LoopVisualiser.h>
#include <FaceModelViewer.h>
using FaceTools::Vis::LoopVisualiser;
using FaceTools::Vis::LoopView;
using FaceTools::Vis::FV;


bool LoopVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _views.count(fv) > 0)
        for ( const LoopView *lview : _views.at(fv))
            if ( lview->belongs(p))
                return true;
    return false;
}   // end belongs


bool LoopVisualiser::isVisible( const FV *fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).at(0)->isVisible();
}   // end isVisible


void LoopVisualiser::syncWithViewTransform( const FV *fv)
{
    assert( _views.count(fv) > 0);
    for ( LoopView *lv : _views.at(fv))
        lv->pokeTransform( fv->transformMatrix());
}   // end syncWithViewTransform


void LoopVisualiser::setHighlighted( const FV* fv, bool v)
{
    assert( _views.count(fv) > 0);
    const double lw = v ? 5.0 : 1.0;
    for ( LoopView *lv : _views.at(fv))
        lv->setLineWidth( lw);
}   // end setHighlighted


void LoopVisualiser::doPurge( const FV *fv)
{
    for ( LoopView *lv : _views.at(fv))
        delete lv;
    _views.erase(fv);
}   // end doPurge


void LoopVisualiser::doSetVisible( const FV* fv, bool v)
{
    for ( LoopView *lv : _views.at(fv))
        lv->setVisible( v, fv->viewer());
}   // end doSetVisible
