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

#include <BoundingVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
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
        float lw = 3.0f;
        float r = 0.3f;
        float g = 0.4f;
        float b = 0.9f;
        if ( !fv->data()->landmarks().empty())
        {
            r = 0.4f;
            g = 0.9f;
            b = 0.3f;
        }   // end if

        _views[fv] = new BoundingView( *fv->data()->bounds()[0], lw, r, g, b);
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


void BoundingVisualisation::syncActorsToData( const FV* fv, const cv::Matx44d& d)
{
    if ( _views.count(fv) > 0)
    {
        const cv::Matx44d& bmat = fv->data()->bounds()[0]->transformMatrix();
        _views.at(fv)->pokeTransform( RVTK::toVTK( d * bmat));
    }   // end if
}   // end syncActorsToData


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
