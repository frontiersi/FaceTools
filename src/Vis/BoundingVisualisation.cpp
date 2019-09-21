/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Vis::FV;
using FaceTools::Action::Event;

BoundingVisualisation::~BoundingVisualisation()
{
    while (!_views.empty())
        purge(const_cast<FV*>(_views.begin()->first), Event::NONE);
}   // end dtor


void BoundingVisualisation::apply( FV* fv, const QPoint*)
{
    if (_views.count(fv) == 0)
    {
        _views[fv] = new BoundingView( fv->data(), 3.0f);
        _setColour(fv);
    }   // end if
    _views.at(fv)->setVisible( true, fv->viewer());
}   // end apply


void BoundingVisualisation::setVisible( FV* fv, bool v)
{
    if (_views.count(fv) > 0)
        _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool BoundingVisualisation::isVisible( const FV *fv) const
{
    bool vis = false;
    if (_views.count(fv) > 0)
        vis = _views.at(fv)->visible();
    return vis;
}   // end isVisible


void BoundingVisualisation::syncToViewTransform( const FV* fv, const vtkMatrix4x4* d)
{
    if ( _views.count(fv) > 0)
    {
        _views.at(fv)->pokeTransform( d);
        _setColour(fv);
    }   // end if
}   // end syncToViewTransform


bool BoundingVisualisation::purge( FV* fv, Event)
{
    if (_views.count(fv) > 0)
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
    return true;
}   // end purge


void BoundingVisualisation::_setColour( const FV* fv)
{
    assert(_views.count(fv) > 0);
    BoundingView* bv = _views.at(fv);
    cv::Vec3d col( 0.3, 0.4, 0.9);
    if ( !fv->data()->currentAssessment()->landmarks().empty())
        col = cv::Vec3d( 0.4, 0.9, 0.3);
    bv->setColour( col[0], col[1], col[2], 0.20);
}   // end _setColour
