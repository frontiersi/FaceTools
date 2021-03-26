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

#include <Action/ActionSetMaxScalarColour.h>
#include <Vis/ColourVisualisation.h>
#include <Vis/FaceView.h>
#include <QColorDialog>
using FaceTools::Action::ActionSetMaxScalarColour;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::Vis::ColourVisualisation;
using MS = FaceTools::ModelSelect;


ActionSetMaxScalarColour::ActionSetMaxScalarColour( const QString& dname)
    : FaceAction( dname)
{
    setIconColour( QColor(255,0,0));
    addRefreshEvent( Event::VIEW_CHANGE);
}   // end ctor


bool ActionSetMaxScalarColour::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    const ColourVisualisation *smapper = fv ? fv->activeColours() : nullptr;
    if ( smapper)
        setIconColour( smapper->maxColour());
    return smapper != nullptr;
}   // end isAllowed


bool ActionSetMaxScalarColour::doBeforeAction( Event)
{
    QColorDialog::ColorDialogOptions options;
    QColor c = QColorDialog::getColor( _curColour, static_cast<QWidget*>(parent()),
                    tr("Choose new maximum surface mapping colour"), options);
    if ( c.isValid())
        setIconColour( c);
    return c.isValid();
}   // end doBeforeAction


void ActionSetMaxScalarColour::doAction( Event)
{
    FV *fv = MS::selectedView();
    ColourVisualisation *cv = fv->activeColours();
    cv->setMaxColour( _curColour);
    cv->rebuildColourMapping();
}   // end doAction


void ActionSetMaxScalarColour::setIconColour( const QColor& colour)
{
    _curColour = colour;
    QPixmap pmap(40,40);
    pmap.fill( colour);
    qaction()->setIcon( pmap);
}   // end setIconColour
