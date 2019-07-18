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

#include <ActionSetMinScalarColour.h>
#include <SurfaceMetricsMapper.h>
#include <FaceView.h>
#include <QColorDialog>
#include <algorithm>
using FaceTools::Action::ActionSetMinScalarColour;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using SMM = FaceTools::Vis::SurfaceMetricsMapper;


ActionSetMinScalarColour::ActionSetMinScalarColour( const QString& dname)
    : FaceAction( dname)
{
    setIconColour( QColor(0,0,255));
}   // end ctor


bool ActionSetMinScalarColour::checkEnable( Event)
{
    const FV* fv = ModelSelector::selectedView();
    const SMM* smm = fv ? fv->activeSurface() : nullptr;
    if ( smm)
        setIconColour( smm->minColour());
    return smm && smm->isScalarMapping();
}   // end checkEnabled


bool ActionSetMinScalarColour::doBeforeAction( Event)
{
    QColor c = QColorDialog::getColor( _curColour, static_cast<QWidget*>(parent()), "Choose new minimum surface mapping colour");
    if ( c.isValid())
        setIconColour( c);
    return c.isValid();
}   // end doBeforeAction


void ActionSetMinScalarColour::doAction( Event)
{
    SMM* smm = ModelSelector::selectedView()->activeSurface();
    assert(smm);
    smm->setMinColour( _curColour);
    smm->rebuild();
    emit onEvent( Event::VIEW_CHANGE);
}   // end doAction


void ActionSetMinScalarColour::setIconColour( const QColor& colour)
{
    _curColour = colour;
    QPixmap pmap(40,40);
    pmap.fill( colour);
    qaction()->setIcon( pmap);
}   // end setIconColour
