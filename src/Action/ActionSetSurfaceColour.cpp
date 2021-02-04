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

#include <Action/ActionSetSurfaceColour.h>
#include <Vis/FaceView.h>
#include <QColorDialog>
using FaceTools::Action::ActionSetSurfaceColour;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using MS = FaceTools::ModelSelect;


ActionSetSurfaceColour::ActionSetSurfaceColour( const QString& dname)
    : FaceAction( dname)
{
    setIconColour( QColor(255,255,255));
}   // end ctor


bool ActionSetSurfaceColour::isAllowed( Event)
{
    if ( MS::isViewSelected())
        setIconColour( MS::selectedView()->colour());
    return MS::isViewSelected();
}   // end isAllowedd


bool ActionSetSurfaceColour::doBeforeAction( Event)
{
    QColor c = QColorDialog::getColor( _curColour, static_cast<QWidget*>(parent()), "Choose new surface colour");
    if ( c.isValid())
        setIconColour( c);
    return c.isValid();
}   // end doBeforeAction


void ActionSetSurfaceColour::doAction( Event)
{
    MS::selectedView()->setColour( _curColour);
}   // end doAction


Event ActionSetSurfaceColour::doAfterAction( Event) { return Event::VIEW_CHANGE;}


void ActionSetSurfaceColour::setIconColour( const QColor& colour)
{
    _curColour = colour;
    QPixmap pmap(40,40);
    pmap.fill( colour);
    qaction()->setIcon( pmap);
}   // end setIconColour
