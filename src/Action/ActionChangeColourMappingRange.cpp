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

#include <Action/ActionChangeColourMappingRange.h>
#include <Vis/ColourVisualisation.h>
#include <Vis/FaceView.h>
#include <QSignalBlocker>
#include <cassert>
using FaceTools::Action::ActionChangeColourMappingRange;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::ColourVisualisation;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionChangeColourMappingRange::ActionChangeColourMappingRange( const QString& dname)
    : FaceAction( dname), _widget(nullptr)
{
    addRefreshEvent( Event::VIEW_CHANGE);
}   // end ctor


QString ActionChangeColourMappingRange::toolTip() const
{
    QStringList txt;
    txt << "Change the range over which the selected surface";
    txt << "visualisation is applied.";
    return txt.join(" ");
}   // end toolTip


void ActionChangeColourMappingRange::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _widget = new QTools::RangeMinMax(p);
    _widget->setToolTip( toolTip());
    _widget->setMinHidden(true);
    _widget->setRange( 0, 1);
    _widget->setMin( 0);
    _widget->setMax( 1);
    _widget->setNumDecimals(1);
    _widget->setEnabled(false);
    _widget->setMinimumWidth(50);   // Double if min spin box visible
    connect( _widget, &QTools::RangeMinMax::valueChanged,
                this, &ActionChangeColourMappingRange::_doOnValueChanged);
}   // end postInit


bool ActionChangeColourMappingRange::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    return fv && fv->activeColours();
}   // end isAllowed


bool ActionChangeColourMappingRange::update( Event)
{
    QSignalBlocker block(_widget);
    const FV* fv = MS::selectedView();
    const ColourVisualisation* svis = fv ? fv->activeColours() : nullptr;
    const bool isEnabled = svis != nullptr;
    if ( isEnabled)
    {
        const float rmin = svis->minRange();
        const float rmax = svis->maxRange();
        _widget->setRange( rmin, rmax);
        const float vmin = svis->minVisible();
        const float vmax = svis->maxVisible();
        assert( vmin <= vmax);
        _widget->setMin( vmin);
        _widget->setMax( vmax);
        const float ss = svis->stepSize();
        _widget->setSingleStepSize( ss);
    }   // end if
    _widget->setEnabled( isEnabled);
    return isEnabled;
}   // end update


void ActionChangeColourMappingRange::_doOnValueChanged( float minv, float maxv)
{
    assert( isEnabled());
    FV* fv = MS::selectedView();
    assert( fv);
    assert( fv->activeColours());
    ColourVisualisation *svis = fv->activeColours();
    svis->setVisibleRange( minv, maxv);
    svis->rebuildColourMapping();
    emit onEvent( Event::VIEW_CHANGE);
}   // end _doOnValueChanged
