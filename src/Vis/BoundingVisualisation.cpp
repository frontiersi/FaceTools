/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceControl.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Vis::BoundingView;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::ModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


BoundingVisualisation::BoundingVisualisation( const QString& dname)
    : BaseVisualisation( dname)
{
}   // end ctor


BoundingVisualisation::~BoundingVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


bool BoundingVisualisation::apply( const FaceControl* fc, const QPoint*)
{
    if ( _views.count(fc) == 0)
        _views[fc] = new BoundingView( fc->data()->bounds());
    return true;
}   // end apply


void BoundingVisualisation::addActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
        _views.at(fc)->setVisible( true, fc->viewer());
}   // end addActors


void BoundingVisualisation::removeActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
        _views.at(fc)->setVisible( false, fc->viewer());
}   // end removeActors


void BoundingVisualisation::setHighlighted( const FaceControl* fc, int c, bool v)
{
    if (_views.count(fc) > 0)
    {
        if ( c >= 0)
            _views.at(fc)->setHighlighted( c, v);
        else
            _views.at(fc)->setHighlighted( v);
    }   // end if
}   // end setHighlighted


// protected
void BoundingVisualisation::pokeTransform( const FaceControl* fc, const vtkMatrix4x4* vm)
{
    if ( _views.count(fc) > 0)
        _views.at(fc)->pokeTransform(vm);
}   // end pokeTransform


// protected
void BoundingVisualisation::fixTransform( const FaceControl* fc)
{
    // Instead of fixing the actor matrix in, new bounds are
    // generated so that the bounding box always remains upright at rest.
    if ( _views.count(fc) > 0)
        _views.at(fc)->updateBounds(fc->data()->bounds());
}   // end fixTransform


// protected
void BoundingVisualisation::purge( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        removeActors(fc);
        delete _views.at(fc);
        _views.erase(fc);
    }   // end if
}   // end purge
