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

#include <Vis/PlaneVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Vis::PlaneVisualisation;
using FaceTools::Vis::FV;


PlaneVisualisation::PlaneVisualisation( int axis)
    : _axis(std::min( std::max( 0, axis), 2)), _col(Vec3f::Zero())
{
    _col[_axis] = 1.0f;
}   // end axis


bool PlaneVisualisation::isAvailable( const FV *fv) const { return fv->data()->hasLandmarks();}


void PlaneVisualisation::refresh( FV* fv)
{
    PlaneView &plane = _views[fv];
    plane.setColour( _col[0], _col[1], _col[2], 0.5);
    const Mat4f I = Mat4f::Identity();
    const Vec3f centre = I.block<3,1>(0,3);
    const Vec3f normal = I.block<3,1>(0,_axis);
    plane.update( centre, normal);
}   // end refresh


void PlaneVisualisation::purge( const FV* fv)
{
    if (_views.count(fv) > 0)
        _views.erase(fv);
}   // end purge


void PlaneVisualisation::setVisible( FV* fv, bool v)
{
    _views[fv].setVisible( v, fv->viewer());
}   // end setVisible


bool PlaneVisualisation::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).isVisible();
}   // end isVisible


void PlaneVisualisation::syncTransform( const FV* fv)
{
    _views[fv].pokeTransform(fv->transformMatrix());
}   // end syncTransform
