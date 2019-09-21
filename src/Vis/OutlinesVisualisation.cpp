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

#include <Vis/OutlinesVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Random.h> // rlib
#include <cassert>
using FaceTools::Vis::OutlinesVisualisation;
using FaceTools::Vis::LoopView;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Action::Event;


OutlinesVisualisation::~OutlinesVisualisation()
{
    while (!_views.empty())
        purge( const_cast<FV*>(_views.begin()->first), Event::NONE);
}   // end dtor


void OutlinesVisualisation::apply( FV* fv, const QPoint*)
{
    if ( _views.count(fv) == 0)
    {
        const FM* fm = fv->data();
        const RFeatures::ObjModel& model = fm->model();
        const RFeatures::ObjModelManifolds& manifolds = fm->manifolds();

        rlib::Random rng(5);
        LoopView* loops = _views[fv] = new LoopView( model, 4.0f);

        const int nm = static_cast<int>(manifolds.count());
        for ( int i = 0; i < nm; ++i)
        {
            // Define a different colour for the boundaries of each manifold
            const double r = 0.2 + 0.8 * rng.getRandom();
            const double g = 0.2 + 0.8 * rng.getRandom();
            const double b = 0.2 + 0.8 * rng.getRandom();

            // Get the boundaries for manifold i
            const RFeatures::ObjModelManifoldBoundaries& bnds = manifolds.manifold(i)->boundaries(model);
            const int nb = static_cast<int>(bnds.count());
            for ( int j = 0; j < nb; ++j)
                loops->add( bnds.boundary(j), r, g, b, 0.99); // vertex positions for boundary j
        }   // end for
    }   // end if
    assert( fv->viewer());
    _views.at(fv)->setVisible( true, fv->viewer());
}   // end apply


bool OutlinesVisualisation::purge( FV* fv, Event)
{
    if (_views.count(fv) > 0)
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
    return true;
}   // end purge


void OutlinesVisualisation::setVisible( FV* fv, bool v)
{
    if (_views.count(fv) > 0)
        _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool OutlinesVisualisation::isVisible( const FV* fv) const
{
    bool vis = false;
    if (_views.count(fv) > 0)
        vis = _views.at(fv)->visible();
    return vis;
}   // end isVisible


void OutlinesVisualisation::syncToViewTransform( const FV* fv, const vtkMatrix4x4* d)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->pokeTransform(d);
}   // end syncToViewTransform
