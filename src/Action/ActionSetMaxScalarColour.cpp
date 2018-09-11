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

#include <ActionSetMaxScalarColour.h>
#include <FaceView.h>
#include <QColorDialog>
#include <algorithm>
using FaceTools::Action::ActionSetMaxScalarColour;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::Vis::ScalarMapping;


ActionSetMaxScalarColour::ActionSetMaxScalarColour( const QString& dname, QWidget* parent)
    : FaceAction( dname), _parent(parent)
{
    setIconColour( QColor(0,0,180));
}   // end ctor


void ActionSetMaxScalarColour::tellReady( FV* fv, bool v)
{
    if ( v)
    {
        ScalarMapping* scmap = fv->activeScalars();
        if ( scmap)
            setIconColour( scmap->maxColour());
    }   // end if
}   // end tellReady


bool ActionSetMaxScalarColour::testReady( const FV* fv) { return fv->activeScalars() != nullptr;}


bool ActionSetMaxScalarColour::doBeforeAction( FVS&, const QPoint&)
{
    QColor c = QColorDialog::getColor( _curColour, _parent, "Choose new maximum scalar colour");
    if ( c.isValid())
        setIconColour( c);
    return c.isValid();
}   // end doBeforeAction


bool ActionSetMaxScalarColour::doAction( FVS& fvs, const QPoint&)
{
    std::unordered_set<ScalarMapping*> scmaps;
    for ( FV* fv : fvs)
    {
        ScalarMapping* scmap = fv->activeScalars();
        assert(scmap);
        scmap->setMaxColour( _curColour);
        scmaps.insert(scmap);
    }   // end for

    std::for_each( std::begin(scmaps), std::end(scmaps), []( auto sc){ sc->rebuild();});
    return true;
}   // end doAction


void ActionSetMaxScalarColour::setIconColour( const QColor& colour)
{
    _curColour = colour;
    QPixmap pmap(40,40);
    pmap.fill( colour);
    qaction()->setIcon( pmap);
}   // end setIconColour
