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

#include <ActionSetNumScalarColours.h>
#include <SurfaceDataMapper.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Action::ActionSetNumScalarColours;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::Vis::SurfaceDataMapper;


ActionSetNumScalarColours::ActionSetNumScalarColours( const QString& dname, QWidget* parent)
    : FaceAction( dname), _spinBox(new QSpinBox(parent))
{
    _spinBox->setAlignment( Qt::AlignRight);
    _spinBox->setRange( 2, 80);
    _spinBox->setSingleStep( 1);
    _spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
}   // end ctor


bool ActionSetNumScalarColours::testReady( const FV* fv)
{
    SurfaceDataMapper* sdm = fv->activeSurface();
    return sdm && sdm->isScalarMapping();
}   // end testReady


void ActionSetNumScalarColours::tellReady( const FV* fv, bool v)
{
    _spinBox->disconnect(this);
    _spinBox->setValue( 0);
    if ( v)
    {
        _spinBox->setValue( (int)fv->activeSurface()->numColours());
        connect( _spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ActionSetNumScalarColours::updateFaceViewFromWidget);
    }   // end if
}   // end tellReady


void ActionSetNumScalarColours::updateFaceViewFromWidget( int v)
{
    assert( isEnabled());
    FV* fv = ready().first();
    assert(fv);
    SurfaceDataMapper* sdm = fv->activeSurface();
    assert(sdm);
    sdm->setNumColours( v);
    sdm->rebuild();
}   // end updateFaceViewFromWidget

