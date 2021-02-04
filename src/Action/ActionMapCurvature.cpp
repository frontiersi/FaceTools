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

#include <Action/ActionMapCurvature.h>
#include <FaceModelCurvatureStore.h>
using FaceTools::Action::ActionMapCurvature;
using FaceTools::Action::Event;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionMapCurvature::ActionMapCurvature() : FaceAction( "Map Curvature")
{
    addPurgeEvent( Event::MESH_CHANGE);
    addTriggerEvent( Event::MESH_CHANGE);
#ifdef NDEBUG
    setAsync(true);
#endif
}   // end ctor


bool ActionMapCurvature::doBeforeAction( Event) { return true;}


void ActionMapCurvature::doAction( Event e)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    FaceModelCurvatureStore::add( *fm);
    for ( FV *fv : fm->fvs())
        fv->resetNormals();
}   // end doAction


Event ActionMapCurvature::doAfterAction( Event)
{
    return Event::SURFACE_DATA_CHANGE;
}   // end doAfterAction


void ActionMapCurvature::purge( const FM* fm) { FaceModelCurvatureStore::purge(*fm);}
