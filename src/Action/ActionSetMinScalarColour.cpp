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

#include <ActionSetMinScalarColour.h>
#include <SurfaceDataMapper.h>
#include <FaceView.h>
#include <QColorDialog>
#include <algorithm>
using FaceTools::Action::ActionSetMinScalarColour;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using SDM = FaceTools::Vis::SurfaceDataMapper;


ActionSetMinScalarColour::ActionSetMinScalarColour( const QString& dname, QWidget* parent)
    : FaceAction( dname), _parent(parent)
{
    setIconColour( QColor(0,0,255));
}   // end ctor


void ActionSetMinScalarColour::tellReady( const FV* fv, bool v)
{
    if ( v)
    {
        SDM* sdm = fv->activeSurface();
        if ( sdm)
            setIconColour( sdm->minColour());
    }   // end if
}   // end tellReady


bool ActionSetMinScalarColour::testReady( const FV* fv)
{
    SDM* sdm = fv->activeSurface();
    return sdm && sdm->isScalarMapping();
}   // end testReady


bool ActionSetMinScalarColour::doBeforeAction( FVS&, const QPoint&)
{
    QColor c = QColorDialog::getColor( _curColour, _parent, "Choose new minimum surface mapping colour");
    if ( c.isValid())
        setIconColour( c);
    return c.isValid();
}   // end doBeforeAction


bool ActionSetMinScalarColour::doAction( FVS& fvs, const QPoint&)
{
    std::unordered_set<SDM*> sdms;
    for ( FV* fv : fvs)
    {
        SDM* sdm = fv->activeSurface();
        assert(sdm);
        sdm->setMinColour( _curColour);
        sdms.insert(sdm);
    }   // end for
    std::for_each( std::begin(sdms), std::end(sdms), [](SDM* sdm){ sdm->rebuild();});
    return true;
}   // end doAction


void ActionSetMinScalarColour::setIconColour( const QColor& colour)
{
    _curColour = colour;
    QPixmap pmap(40,40);
    pmap.fill( colour);
    qaction()->setIcon( pmap);
}   // end setIconColour
