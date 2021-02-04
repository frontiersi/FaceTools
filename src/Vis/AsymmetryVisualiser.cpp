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

#include <Vis/AsymmetryVisualiser.h>
#include <Metric/AsymmetryMetricType.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::AsymmetryVisualiser;
using FaceTools::Vis::FV;


bool AsymmetryVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _views.count(fv) > 0)
        for ( const AsymmetryView &aview : _views.at(fv))
            if ( aview.belongs(p))
                return true;
    return false;
}   // end belongs


bool AsymmetryVisualiser::isVisible( const FV *fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).at(0).isVisible();
}   // end isVisible


void AsymmetryVisualiser::syncTransform( const FV* fv)
{
    assert( _views.count(fv) > 0);
    if ( _views.count(fv) > 0)
        for ( AsymmetryView &aview : _views.at(fv))
            aview.pokeTransform( fv->transformMatrix());
}   // end syncTransform


void AsymmetryVisualiser::setHighlighted( bool v)
{
    const double lw = v ? 3.0 : 1.0;
    for ( auto &p : _views)
        for ( AsymmetryView &aview : p.second)
            aview.setLineWidth( lw);
}   // end setHighlighted


void AsymmetryVisualiser::purge( const FV *fv) { _views.erase(fv);}


void AsymmetryVisualiser::setVisible( FV* fv, bool v)
{
    if ( _views.count(fv) > 0)
        for ( AsymmetryView &aview : _views.at(fv))
            aview.setVisible( v, fv->viewer());
}   // end setVisible


void AsymmetryVisualiser::refresh( FV *fv)
{
    const Metric::AsymmetryMetricType *ametric = static_cast<const Metric::AsymmetryMetricType*>(metric());
    const std::vector<Metric::AsymmetryMeasure> &ainfo = ametric->asymmetryInfo(fv->data());
    std::vector<AsymmetryView> &views = _views[fv];
    views.resize( ainfo.size());
    for ( size_t i = 0; i < ainfo.size(); ++i)
    {
        const Metric::AsymmetryMeasure &am = ainfo.at(i);
        AsymmetryView &aview = views.at(i);
        aview.setLineColour( 0.1, 0.4, 0.7, 0.99);
        aview.setArrowColour( 0.7, 0.5, 0.1, 0.99);
        aview.update( am.point0, am.point1, am.delta);
    }   // end for
}   // end refresh

