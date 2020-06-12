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

#include <Action/ActionSetNumScalarColours.h>
#include <Vis/ScalarVisualisation.h>
#include <Vis/FaceView.h>
#include <QSignalBlocker>
#include <cassert>
using FaceTools::Action::ActionSetNumScalarColours;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::Vis::ScalarVisualisation;
using MS = FaceTools::Action::ModelSelector;


ActionSetNumScalarColours::ActionSetNumScalarColours( const QString& dname)
    : FaceAction( dname), _spinBox(nullptr) {}


void ActionSetNumScalarColours::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _spinBox = new QSpinBox(p);
    _spinBox->setAlignment( Qt::AlignRight);
    _spinBox->setRange( 1, 99);
    _spinBox->setSingleStep( 1);
    _spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _spinBox->setEnabled(false);
    connect( _spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ActionSetNumScalarColours::_doOnValueChanged);
}   // end postInit


bool ActionSetNumScalarColours::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    return fv && fv->activeScalars();
}   // end isAllowed


bool ActionSetNumScalarColours::checkState( Event)
{
    const FV* fv = MS::selectedView();
    const ScalarVisualisation* svis = fv ? fv->activeScalars() : nullptr;
    const bool enabled = svis != nullptr;
    _spinBox->setEnabled( enabled);
    QSignalBlocker blocker( _spinBox);
    _spinBox->setValue( enabled ? static_cast<int>(svis->numColours()) : 0);
    _spinBox->setSingleStep( enabled ? static_cast<int>(svis->numStepSize()) : 1);
    return enabled;
}   // end checkState


void ActionSetNumScalarColours::_doOnValueChanged( int v)
{
    assert( isEnabled());
    FV* fv = MS::selectedView();
    assert(fv);
    ScalarVisualisation* svis = fv->activeScalars();
    assert(svis);
    svis->setNumColours( static_cast<size_t>(v));
    svis->rebuild();
    fv->setActiveScalars( svis);
    emit onEvent( Event::VIEW_CHANGE);
}   // end _doOnValueChanged

