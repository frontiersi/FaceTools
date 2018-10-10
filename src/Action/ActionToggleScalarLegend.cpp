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

#include <ActionToggleScalarLegend.h>
#include <SurfaceDataMapper.h>
#include <FaceModelViewer.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionToggleScalarLegend;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::SurfaceDataMapper;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FVS;


ActionToggleScalarLegend::ActionToggleScalarLegend( const QString& dn, FMV* v)
    : FaceAction( dn)
{
    if ( v)
        addViewer(v);
    setCheckable(true,true);
    setRespondToEvent( VIEW_CHANGE, [this](const FVS&){ return this->isChecked();});
    setRespondToEvent( VIEWER_CHANGE, [this](const FVS&){ return this->isChecked();});
}   // end ctor


bool ActionToggleScalarLegend::doAction( FVS&, const QPoint&)
{
    if ( !isChecked())
    {
        std::for_each( std::begin(_viewers), std::end(_viewers), [](FMV* v){ v->showLegend(false); v->updateRender();});
        return true;
    }   // end if

    for ( FMV *vwr : _viewers)
    {
        SurfaceDataMapper *sdm = nullptr;
        for ( FV *fv : vwr->attached())
        {
            if ( (sdm = fv->activeSurface()))
                break;
        }   // end for

        bool showLegend = false;
        if ( sdm && sdm->isScalarMapping())
        {
            vwr->setLegend( sdm->label(), sdm->scalarLookupTable());
            showLegend = true;
        }   // end if

        vwr->showLegend( showLegend);
        vwr->updateRender();
    }   // end for
    return true;
}   // end doAction
