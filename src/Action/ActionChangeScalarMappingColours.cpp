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

#include <ActionChangeScalarMappingColours.h>
#include <FaceView.h>
#include <cmath>
#include <cassert>
using FaceTools::Action::ActionChangeScalarMappingColours;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::ScalarMapping;
using FaceTools::Vis::FV;


ActionChangeScalarMappingColours::ActionChangeScalarMappingColours( const QString& dname, QWidget* parent)
    : FaceAction( dname), _widget(new QTools::RangeMinMax(parent))
{
    _widget->setRange( 0, 1);
    _widget->setMin( 0);
    _widget->setMax( 1);
    _widget->setNumDecimals(2);
}   // end ctor


bool ActionChangeScalarMappingColours::testReady( const FV* fv) { return fv->activeScalars() != nullptr;}


void ActionChangeScalarMappingColours::tellReady( FV* fv, bool v)
{
    _widget->disconnect(this);
    if ( v)
    {
        ScalarMapping* scmap = fv->activeScalars();

        const float rmin = scmap->minRange();
        const float rmax = scmap->maxRange();
        _widget->setRange( rmin, rmax);

        const float vmin = scmap->minVisible();
        const float vmax = scmap->maxVisible();

        assert( vmin <= vmax);
        _widget->setMin( vmin);
        _widget->setMax( vmax);

        const float ss = (rmax - rmin) / 20;
        _widget->setSingleStepSize( ss);

        connect( _widget, &QTools::RangeMinMax::rangeChanged, this, &ActionChangeScalarMappingColours::doOnWidgetRangeChanged);
    }   // end if
}   // end tellReady


void ActionChangeScalarMappingColours::doOnWidgetRangeChanged( float minv, float maxv)
{
    assert( isEnabled());
    FV* fv = ready().first();
    assert( fv);
    ScalarMapping* scmap = fv->activeScalars();
    assert(scmap);

    // Update fv's scalar colour mapper from the widget. Note that the scalar colour mapper
    // object is shared between all FaceViews that have this visualisation set, so this will
    // have the effect of updating all of those FaceViews.
    scmap->setVisibleLimits( minv, maxv);
    scmap->rebuild();   // Will cause ScalarMapper::rebuilt to fire on the views that have it.
}   // end doOnWidgetRangeChanged
