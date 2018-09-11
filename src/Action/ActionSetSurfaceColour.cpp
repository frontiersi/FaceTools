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

#include <ActionSetSurfaceColour.h>
#include <FaceView.h>
#include <QColorDialog>
using FaceTools::Action::ActionSetSurfaceColour;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using FaceTools::FVS;


ActionSetSurfaceColour::ActionSetSurfaceColour( const QString& dname, QWidget* parent)
    : FaceAction( dname), _parent(parent)
{
    setIconColour( QColor(255,255,255));
}   // end ctor


void ActionSetSurfaceColour::tellReady( FV* fv, bool v)
{
    if ( v)
        setIconColour( fv->colour());
}   // end tellReady


bool ActionSetSurfaceColour::doBeforeAction( FVS&, const QPoint&)
{
    QColor c = QColorDialog::getColor( _curColour, _parent, "Choose new surface colour");
    if ( c.isValid())
        setIconColour( c);
    return c.isValid();
}   // end doBeforeAction


bool ActionSetSurfaceColour::doAction( FVS& fvs, const QPoint&)
{
    for ( FV* fv : fvs)
        fv->setColour( _curColour);
    return true;
}   // end doAction


void ActionSetSurfaceColour::setIconColour( const QColor& colour)
{
    _curColour = colour;
    QPixmap pmap(40,40);
    pmap.fill( colour);
    qaction()->setIcon( pmap);
}   // end setIconColour
