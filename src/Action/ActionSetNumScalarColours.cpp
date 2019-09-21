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

#include <Action/ActionSetNumScalarColours.h>
#include <Vis/SurfaceMetricsMapper.h>
#include <Vis/FaceView.h>
#include <cassert>
using FaceTools::Action::ActionSetNumScalarColours;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::Vis::SurfaceMetricsMapper;


ActionSetNumScalarColours::ActionSetNumScalarColours( const QString& dname)
    : FaceAction( dname), _spinBox(nullptr) {}


void ActionSetNumScalarColours::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _spinBox = new QSpinBox(p);
    _spinBox->setAlignment( Qt::AlignRight);
    _spinBox->setRange( 2, 99);
    _spinBox->setSingleStep( 1);
    _spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _spinBox->setEnabled(false);
}   // end postInit


bool ActionSetNumScalarColours::checkEnable( Event)
{
    _spinBox->setEnabled(false);
    _spinBox->disconnect(this);
    _spinBox->setValue( 0);
    const FV* fv = ModelSelector::selectedView();
    const SurfaceMetricsMapper* smm = fv ? fv->activeSurface() : nullptr;
    const bool enabled = smm && smm->isScalarMapping();
    if ( enabled)
    {
        _spinBox->setEnabled(true);
        _spinBox->setValue( static_cast<int>(fv->activeSurface()->numColours()));
        connect( _spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &ActionSetNumScalarColours::updateFaceViewFromWidget);
    }   // end if
    return enabled;
}   // end checkEnabled


void ActionSetNumScalarColours::updateFaceViewFromWidget( int v)
{
    assert( isEnabled());
    const FV* fv = ModelSelector::selectedView();
    assert(fv);
    SurfaceMetricsMapper* smm = fv->activeSurface();
    assert(smm);
    smm->setNumColours( static_cast<size_t>(v));
    smm->rebuild();
}   // end updateFaceViewFromWidget

