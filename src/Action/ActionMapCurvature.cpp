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

#include <Action/ActionMapCurvature.h>
#include <FaceModelCurvature.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionMapCurvature;
using FaceTools::Action::Event;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;
using FMC = FaceTools::FaceModelCurvature;


ActionMapCurvature::ActionMapCurvature() : FaceAction( "Map Curvature")
{
    addPurgeEvent( Event::MESH_CHANGE);
    addTriggerEvent( Event::MESH_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionMapCurvature::isAllowed( Event) { return MS::selectedModel();}


void ActionMapCurvature::doAction( Event e)
{
    const FM* fm = MS::selectedModel();
    fm->lockForRead();
    FMC::purge(fm);
    FMC::add(fm);
    fm->unlock();
}   // end doAction


Event ActionMapCurvature::doAfterAction( Event)
{
    const FM *fm = MS::selectedModel();
    for ( FV *fv : fm->fvs())
        fv->resetNormals();
    return Event::SURFACE_DATA_CHANGE;
}   // end doAfterAction


void ActionMapCurvature::purge( const FM* fm) { FMC::purge(fm);}
