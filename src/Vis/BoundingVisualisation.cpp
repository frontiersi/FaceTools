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

#include <Vis/BoundingVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Vis::FV;

BoundingVisualisation::~BoundingVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


void BoundingVisualisation::apply( const FV* fv, const QPoint*)
{
    if (_views.count(fv) == 0)
        _views[fv] = new BoundingView( 2.0f);
}   // end apply


void BoundingVisualisation::setVisible( FV* fv, bool v)
{
    if (_views.count(fv) > 0)
        _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool BoundingVisualisation::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv)->isVisible();
}   // end isVisible


void BoundingVisualisation::syncWithViewTransform( const FV* fv)
{
    if ( _views.count(fv) > 0)
    {
        fv->data()->lockForRead();
        _setAppearance(fv);
        fv->data()->unlock();
        _views.at(fv)->pokeTransform( fv->transformMatrix());
    }   // end if
}   // end syncWithViewTransform


void BoundingVisualisation::_setAppearance( const FV* fv)
{
    static const Vec3f DEFAULT_COL( 0.0f, 0.2f, 0.9f);  // Default colour (blue)
    static const Vec3f LNDMRK_COL( 0.0f, 0.9f, 0.2f);    // Green

    const FM *fm = fv->data();
    const Vec3f col = fm->hasLandmarks() ? LNDMRK_COL : DEFAULT_COL;
    BoundingView *bv = _views.at(fv);
    bv->update( fm->bounds()[0]->cornersAs6f());
    bv->setColour( col[0], col[1], col[2], 0.32f);
    bv->setLineStipplingEnabled( !fm->isAligned());
}   // end _setAppearance


void BoundingVisualisation::purge( const FV* fv)
{
    if (_views.count(fv) > 0)
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
}   // end purge
