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
#include <FaceModelViewer.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionToggleScalarLegend;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::ScalarMapping;
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
}   // end ctor


bool ActionToggleScalarLegend::doAction( FVS& fvs, const QPoint&)
{
    // Legends default to not being shown across all viewers.
    std::for_each( std::begin(_viewers), std::end(_viewers), [](auto v){ v->showLegend(false);});
    if ( !isChecked())
        return true;

    std::unordered_map<FMV*, ScalarMapping*> vmaps;
    for ( FV* fv : fvs)
    {
        ScalarMapping* scmap = fv->activeScalars();
        if ( scmap)
        {
            FMV* vwr = fv->viewer();
            if ( vmaps.count(vwr) > 0)
                assert( vmaps.at(vwr) == scmap);    // All FaceViews in the same viewer MUST have the same scalar mapping!
            vmaps[vwr] = scmap;
        }   // end if
    }   // end for

    for ( auto p : vmaps)
    {
        FMV* vwr = p.first;
        ScalarMapping* scmap = p.second;
        vwr->setLegend( scmap->rangeName(), scmap->lookupTable().vtk());
        vwr->showLegend(true);
    }   // end for
    return true;
}   // end doAction
