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

#include <Vis/AsymmetryVisualiser.h>
#include <Metric/AsymmetryMetricType.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::AsymmetryVisualiser;
using FaceTools::Vis::AsymmetryView;
using FaceTools::Vis::FV;


bool AsymmetryVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _views.count(fv) > 0)
        for ( const AsymmetryView *aview : _views.at(fv))
            if ( aview->belongs(p))
                return true;
    return false;
}   // end belongs


bool AsymmetryVisualiser::isVisible( const FV *fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).at(0)->isVisible();
}   // end isVisible


void AsymmetryVisualiser::syncWithViewTransform( const FV* fv)
{
    assert( _views.count(fv) > 0);
    for ( AsymmetryView *aview : _views.at(fv))
        aview->pokeTransform( fv->transformMatrix());
}   // end syncWithViewTransform


void AsymmetryVisualiser::setHighlighted( const FV* fv, bool v)
{
    assert( _views.count(fv) > 0);
    const double lw = v ? 3.0 : 1.0;
    for ( AsymmetryView *aview : _views.at(fv))
        aview->setLineWidth( lw);
}   // end setHighlighted


void AsymmetryVisualiser::doPurge( const FV *fv)
{
    for ( AsymmetryView *aview : _views.at(fv))
        delete aview;
    _views.erase(fv);
}   // end doPurge


void AsymmetryVisualiser::doSetVisible( const FV* fv, bool v)
{
    for ( AsymmetryView *aview : _views.at(fv))
        aview->setVisible( v, fv->viewer());
}   // end doSetVisible


void AsymmetryVisualiser::doApply( const FV *fv)
{
    const Metric::AsymmetryMetricType *ametric = static_cast<const Metric::AsymmetryMetricType*>(metric());
    const std::vector<Metric::AsymmetryMeasure> &ainfo = ametric->asymmetryInfo(fv->data());
    for ( size_t i = 0; i < ainfo.size(); ++i)
    {
        AsymmetryView *aview = new AsymmetryView;
        aview->setLineColour( 0.1, 0.4, 0.7, 0.99);
        aview->setArrowColour( 0.7, 0.5, 0.1, 0.99);
        _views[fv].push_back(aview);
    }   // end for
}   // end doApply


void AsymmetryVisualiser::doRefresh( const FV *fv)
{
    const std::vector<AsymmetryView*> &views = _views.at(fv);
    const Metric::AsymmetryMetricType *ametric = static_cast<const Metric::AsymmetryMetricType*>(metric());
    const std::vector<Metric::AsymmetryMeasure> &ainfo = ametric->asymmetryInfo(fv->data());
    for ( size_t i = 0; i < ainfo.size(); ++i)
    {
        const Metric::AsymmetryMeasure &am = ainfo.at(i);
        views[i]->update( am.point0, am.point1, am.delta);
    }   // end for
}   // end doRefresh

