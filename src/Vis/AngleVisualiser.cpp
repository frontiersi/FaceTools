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

#include <Vis/AngleVisualiser.h>
#include <Metric/AngleMetricType.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::AngleVisualiser;
using FaceTools::Vis::FV;


bool AngleVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _angles.count(fv) > 0)
        for ( const AngleView &angle : _angles.at(fv))
            if ( angle.belongs(p))
                return true;
    return false;
}   // end belongs


bool AngleVisualiser::isVisible( const FV *fv) const
{
    return _angles.count(fv) > 0 && _angles.at(fv).at(0).isVisible();
}   // end isVisible


void AngleVisualiser::syncTransform( const FV* fv)
{
    assert( _angles.count(fv) > 0);
    if ( _angles.count(fv) > 0)
        for ( AngleView &angle : _angles.at(fv))
            angle.pokeTransform( fv->transformMatrix());
}   // end syncTransform


void AngleVisualiser::setHighlighted( bool v)
{
    const double lw = v ? 5.0 : 1.0;
    for ( auto &p : _angles)
        for ( AngleView &angle : p.second)
            angle.setLineWidth( lw);
}   // end setHighlighted


void AngleVisualiser::purge( const FV *fv) { _angles.erase(fv);}


void AngleVisualiser::refresh( FV *fv)
{
    const Metric::AngleMetricType *ametric = static_cast<const Metric::AngleMetricType*>(metric());
    const std::vector<Metric::AngleMeasure> &ainfo = ametric->angleInfo(fv->data());
    std::vector<AngleView> &aviews = _angles[fv];
    aviews.resize( ainfo.size());
    for ( size_t i = 0; i < ainfo.size(); ++i)
    {
        const Metric::AngleMeasure &am = ainfo.at(i);
        AngleView &angle = aviews.at(i);
        angle.setColour( 0.1, 0.7, 0.0);
        angle.update( am.point0, am.point1, am.centre, am.normal, am.degrees);
    }   // end for
}   // end refresh


void AngleVisualiser::setVisible( FV* fv, bool v)
{
    if ( _angles.count(fv) > 0)
        for ( AngleView &angle : _angles.at(fv))
            angle.setVisible( v, fv->viewer());
}   // end setVisible
