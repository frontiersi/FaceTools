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

#include <ActionChangeSurfaceMappingRange.h>
#include <SurfaceMetricsMapper.h>
#include <FaceView.h>
#include <cmath>
#include <cassert>
using FaceTools::Action::ActionChangeSurfaceMappingRange;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::SurfaceMetricsMapper;
using FaceTools::Vis::FV;


ActionChangeSurfaceMappingRange::ActionChangeSurfaceMappingRange( const QString& dname)
    : FaceAction( dname), _widget(nullptr) { }


void ActionChangeSurfaceMappingRange::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _widget = new QTools::RangeMinMax(p);
    _widget->setRange( 0, 1);
    _widget->setMin( 0);
    _widget->setMax( 1);
    _widget->setNumDecimals(2);
    _widget->setEnabled(false);
    _widget->setMinimumWidth(140);
}   // end postInit


bool ActionChangeSurfaceMappingRange::checkEnable( Event)
{
    _widget->disconnect(this);
    _widget->setEnabled(false);

    const FV* fv = ModelSelector::selectedView();
    const bool isEnabled = fv && fv->activeSurface() != nullptr;

    if ( isEnabled)
    {
        _widget->setEnabled(true);
        SurfaceMetricsMapper* smm = fv->activeSurface();

        const float rmin = smm->minRange();
        const float rmax = smm->maxRange();
        _widget->setRange( rmin, rmax);

        const float vmin = smm->minVisible();
        const float vmax = smm->maxVisible();

        assert( vmin <= vmax);
        _widget->setMin( vmin);
        _widget->setMax( vmax);

        const float ss = (rmax - rmin) / 20;
        _widget->setSingleStepSize( ss);

        connect( _widget, &QTools::RangeMinMax::rangeChanged, this, &ActionChangeSurfaceMappingRange::doOnWidgetRangeChanged);
    }   // end if

    return isEnabled;
}   // end checkEnabled


void ActionChangeSurfaceMappingRange::doOnWidgetRangeChanged( float minv, float maxv)
{
    assert( isEnabled());
    const FV* fv = ModelSelector::selectedView();
    SurfaceMetricsMapper *smm = fv->activeSurface();
    assert(smm);
    // Update fv's scalar colour mapper from the widget. Note that the scalar colour mapper
    // object is shared between all FaceViews that have this visualisation set, so this will
    // have the effect of updating the surfaces of all those FaceViews' actors.
    smm->setVisibleRange( minv, maxv);
    smm->rebuild();
}   // end doOnWidgetRangeChanged
