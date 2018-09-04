/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <BoundingVisualisation.h>
#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Vis::BoundingView;
using FaceTools::Vis::FV;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::ModelViewer;
using FaceTools::FVS;


BoundingVisualisation::BoundingVisualisation( const QString& dname)
    : BaseVisualisation( dname)
{
}   // end ctor


BoundingVisualisation::~BoundingVisualisation()
{
    while (!_views.empty())
        purge(const_cast<FV*>(_views.begin()->first));
}   // end dtor


void BoundingVisualisation::apply( FV* fv, const QPoint*)
{
    if ( _views.count(fv) == 0)
        _views[fv] = new BoundingView( fv->data()->bounds());
    _views.at(fv)->setVisible( true, fv->viewer());
}   // end apply


void BoundingVisualisation::remove( FV* fv)
{
    if (_views.count(fv) > 0)
        _views.at(fv)->setVisible( false, fv->viewer());
}   // end remove


void BoundingVisualisation::setHighlighted( const FV* fv, int c, bool v)
{
    if (_views.count(fv) > 0)
    {
        if ( c >= 0)
            _views.at(fv)->setHighlighted( c, v);
        else
            _views.at(fv)->setHighlighted( v);
    }   // end if
}   // end setHighlighted


// protected
void BoundingVisualisation::pokeTransform( const FV* fv, const vtkMatrix4x4* vm)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->pokeTransform(vm);
}   // end pokeTransform


// protected
void BoundingVisualisation::fixTransform( const FV* fv)
{
    // Instead of fixing the actor matrix in, new bounds are
    // generated so that the bounding box always remains upright at rest.
    if ( _views.count(fv) > 0)
        _views.at(fv)->updateBounds(fv->data()->bounds());
}   // end fixTransform


// protected
void BoundingVisualisation::purge( FV* fv)
{
    if (_views.count(fv) > 0)
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
}   // end purge
