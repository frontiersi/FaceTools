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

#include <Action/ActionSetMaxScalarColour.h>
#include <Vis/ScalarVisualisation.h>
#include <Vis/FaceView.h>
#include <QColorDialog>
using FaceTools::Action::ActionSetMaxScalarColour;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::Vis::ScalarVisualisation;
using MS = FaceTools::Action::ModelSelector;


ActionSetMaxScalarColour::ActionSetMaxScalarColour( const QString& dname)
    : FaceAction( dname)
{
    setIconColour( QColor(255,0,0));
}   // end ctor


bool ActionSetMaxScalarColour::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    const ScalarVisualisation *smapper = fv ? fv->activeScalars() : nullptr;
    if ( smapper)
        setIconColour( smapper->maxColour());
    return smapper != nullptr;
}   // end isAllowedd


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
    ScalarVisualisation *smapper = fv->activeScalars();
    assert(smapper);
    smapper->setMaxColour( _curColour);
    smapper->rebuild();
    fv->setActiveScalars( smapper); // Ensure forwards through
}   // end doAction


void ActionSetMaxScalarColour::setIconColour( const QColor& colour)
{
    _curColour = colour;
    QPixmap pmap(40,40);
    pmap.fill( colour);
    qaction()->setIcon( pmap);
}   // end setIconColour
