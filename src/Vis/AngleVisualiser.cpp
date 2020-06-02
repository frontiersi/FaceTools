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

#include <Vis/AngleVisualiser.h>
#include <Metric/AngleMetricType.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::AngleView;
using FaceTools::Vis::AngleVisualiser;
using FaceTools::Vis::FV;


bool AngleVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _angles.count(fv) > 0)
        for ( const AngleView *angle : _angles.at(fv))
            if ( angle->belongs(p))
                return true;
    return false;
}   // end belongs


bool AngleVisualiser::isVisible( const FV *fv) const
{
    return _angles.count(fv) > 0 && _angles.at(fv).at(0)->isVisible();
}   // end isVisible


void AngleVisualiser::syncWithViewTransform( const FV* fv)
{
    assert( _angles.count(fv) > 0);
    for ( AngleView *angle : _angles.at(fv))
        angle->pokeTransform( fv->transformMatrix());
}   // end syncWithViewTransform


void AngleVisualiser::setHighlighted( const FV* fv, bool v)
{
    assert( _angles.count(fv) > 0);
    const double lw = v ? 5.0 : 1.0;
    for ( AngleView *angle : _angles.at(fv))
        angle->setLineWidth( lw);
}   // end setHighlighted


void AngleVisualiser::doApply( const FV *fv)
{
    const Metric::AngleMetricType *ametric = static_cast<const Metric::AngleMetricType*>(metric());
    const std::vector<Metric::AngleMeasure> &ainfo = ametric->angleInfo(fv->data());
    for ( size_t i = 0; i < ainfo.size(); ++i)
    {
        AngleView *angle = new AngleView;
        angle->setColour( 0.1, 0.7, 0.0);
        _angles[fv].push_back( angle);
    }   // end for
}   // end _applyActor


void AngleVisualiser::doPurge( const FV *fv)
{
    for ( AngleView *angle : _angles.at(fv))
        delete angle;
    _angles.erase(fv);
}   // end doPurge


void AngleVisualiser::doRefresh( const FV *fv)
{
    const std::vector<AngleView*> &aviews = _angles.at(fv);
    const Metric::AngleMetricType *ametric = static_cast<const Metric::AngleMetricType*>(metric());
    const std::vector<Metric::AngleMeasure> &ainfo = ametric->angleInfo(fv->data());
    for ( size_t i = 0; i < ainfo.size(); ++i)
    {
        const Metric::AngleMeasure &am = ainfo.at(i);
        aviews[i]->update( am.point0, am.point1, am.centre, am.normal, am.degrees);
    }   // end for
}   // end doRefresh


void AngleVisualiser::doSetVisible( const FV* fv, bool v)
{
    for ( AngleView *angle : _angles.at(fv))
        angle->setVisible( v, fv->viewer());
}   // end doSetVisible
