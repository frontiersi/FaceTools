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

#include <Vis/MaskVisualisation.h>
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
using FaceTools::Vis::MaskVisualisation;
using FaceTools::Vis::FV;

MaskVisualisation::~MaskVisualisation()
{
    while ( !_views.empty())
        purge( _views.begin()->first);
}   // end dtor


bool MaskVisualisation::isAvailable( const FV *fv) const { return fv->rdata()->hasMask();}


void MaskVisualisation::purge( const FV *fv)
{
    if ( _views.count(fv) > 0)
    {
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
}   // end purge


void MaskVisualisation::setVisible( FV* fv, bool v)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool MaskVisualisation::isVisible( const FV *fv) const
{
    return _views.count(fv) > 0 && _views.at(fv)->isVisible();
}   // end isVisible


void MaskVisualisation::refresh( FV *fv)
{
    if ( _views.count(fv) == 0)
        _views[fv] = new MaskView( fv->rdata()->mask());
    _views.at(fv)->refresh( fv);
}   // end refresh


void MaskVisualisation::syncTransform( const FV *fv)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->pokeTransform(fv->transformMatrix());
}   // end syncTransform
