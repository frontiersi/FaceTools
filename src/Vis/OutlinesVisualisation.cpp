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

#include <Vis/OutlinesVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <rlib/Random.h>
using FaceTools::Vis::OutlinesVisualisation;
using FaceTools::Vis::LoopView;
using FaceTools::Vis::FV;


bool OutlinesVisualisation::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).at(0)->isVisible();
}   // end isVisible


void OutlinesVisualisation::syncWithViewTransform( const FV* fv)
{
    assert( _views.count(fv) > 0);
    for ( LoopView *lv : _views.at(fv))
        lv->pokeTransform( fv->transformMatrix());
}   // end syncWithViewTransform


void OutlinesVisualisation::apply( const FV *fv, const QPoint*)
{
    assert( _views.count(fv) == 0);
    std::vector<LoopView*>& views = _views[fv];
    const FM* fm = fv->data();
    const r3d::Mesh& mesh = fm->mesh();
    const r3d::Manifolds& manifolds = fm->manifolds();

    rlib::Random rng( 5); // Randomize based on model

    for ( size_t i = 0; i < manifolds.count(); ++i)
    {
        // Define a different colour for the boundaries of each manifold
        const double r = 0.2 + 0.8 * rng.getRandom();
        const double g = 0.2 + 0.8 * rng.getRandom();
        const double b = 0.2 + 0.8 * rng.getRandom();

        // Get the boundaries for manifold i
        const r3d::Boundaries& bnds = manifolds[int(i)].boundaries();
        for ( size_t j = 0; j < bnds.count(); ++j)
        {
            LoopView *lv = new LoopView;
            lv->setColour( r, g, b);
            lv->setLineWidth( 3.0);
            views.push_back(lv);

            const std::list<int> &blist = bnds.boundary(int(j));
            std::vector<const Vec3f*> vtxs( blist.size());
            int k = 0;
            for ( int vidx : blist)
                vtxs[k++] = &mesh.uvtx(vidx);
            lv->update( vtxs);
        }   // end for
    }   // end for
}   // end apply


void OutlinesVisualisation::purge( const FV* fv)
{
    assert(_views.count(fv) > 0);
    for ( LoopView *lv : _views.at(fv))
        delete lv;
    _views.erase(fv);
}   // end purge


void OutlinesVisualisation::setVisible( FV* fv, bool v)
{
    assert( _views.count(fv) > 0);
    for ( LoopView *lv : _views.at(fv))
        lv->setVisible( v, fv->viewer());
}   // end setVisible
