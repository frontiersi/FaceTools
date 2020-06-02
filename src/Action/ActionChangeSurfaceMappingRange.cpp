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

#include <Action/ActionChangeSurfaceMappingRange.h>
#include <Vis/ScalarVisualisation.h>
#include <Vis/FaceView.h>
#include <QSignalBlocker>
#include <cassert>
using FaceTools::Action::ActionChangeSurfaceMappingRange;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


ActionChangeSurfaceMappingRange::ActionChangeSurfaceMappingRange( const QString& dname)
    : FaceAction( dname), _widget(nullptr) { }


void ActionChangeSurfaceMappingRange::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _widget = new QTools::RangeMinMax(p);
    _widget->setMinHidden(true);
    _widget->setRange( 0, 1);
    _widget->setMin( 0);
    _widget->setMax( 1);
    _widget->setNumDecimals(1);
    _widget->setEnabled(false);
    _widget->setMinimumWidth(50);   // Double if min spin box visible
    connect( _widget, &QTools::RangeMinMax::valueChanged, this, &ActionChangeSurfaceMappingRange::_doOnWidgetValueChanged);
}   // end postInit


bool ActionChangeSurfaceMappingRange::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    return fv && fv->activeScalars();
}   // end isAllowed


bool ActionChangeSurfaceMappingRange::checkState( Event)
{
    QSignalBlocker block(_widget);
    const FV* fv = MS::selectedView();
    const ScalarVisualisation* svis = fv ? fv->activeScalars() : nullptr;
    const bool isEnabled = svis != nullptr;
    if ( isEnabled)
    {
        const float rmin = svis->minRange();
        const float rmax = svis->maxRange();
        _widget->setRange( rmin, rmax);
        //std::cout << "Vis: " << svis->name() << " range set to " << rmin << ", " << rmax << std::endl;

        const float vmin = svis->minVisible();
        const float vmax = svis->maxVisible();
        assert( vmin <= vmax);
        _widget->setMin( vmin);
        _widget->setMax( vmax);
        //std::cout << "\t\t min, max as " << vmin << ", " << vmax << std::endl;

        const float ss = svis->stepSize();
        _widget->setSingleStepSize( ss);
    }   // end if
    _widget->setEnabled( isEnabled);
    return isEnabled;
}   // end checkState


void ActionChangeSurfaceMappingRange::_doOnWidgetValueChanged( float minv, float maxv)
{
    assert( isEnabled());
    FV* fv = MS::selectedView();
    ScalarVisualisation *svis = fv->activeScalars();
    assert(svis);
    svis->setVisibleRange( minv, maxv);
    svis->rebuild();
    fv->setActiveScalars( svis);
}   // end _doOnWidgetRangeChanged
