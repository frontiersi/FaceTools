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

#include <Action/ActionUpdateU3D.h>
#include <Report/ReportManager.h>
#include <U3DCache.h>
#include <algorithm>
using FaceTools::Action::ActionUpdateU3D;
using FaceTools::Action::Event;
using FaceTools::U3DCache;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


ActionUpdateU3D::ActionUpdateU3D() : FaceAction( "U3D Updater")
{
    addPurgeEvent( Event::MESH_CHANGE);
    addTriggerEvent( Event::MESH_CHANGE | Event::LOADED_MODEL);
    setAsync( true);
}   // end ctor


bool ActionUpdateU3D::isAllowed( Event)
{
    if ( !Report::ReportManager::isAvailable() || !U3DCache::isAvailable())
        return false;
    const FM *fm = MS::selectedModel();
    return fm && fm->hasLandmarks() && fm->isAligned();
}   // end isAllowed


bool ActionUpdateU3D::doBeforeAction( Event e) { return isAllowed(e);}


void ActionUpdateU3D::doAction( Event) { U3DCache::refresh( *MS::selectedModel());}


void ActionUpdateU3D::purge( const FM* fm) { U3DCache::purge(*fm);}


Event ActionUpdateU3D::doAfterAction( Event)
{
#ifndef NDEBUG
    const std::string fpath = U3DCache::u3dfilepath( *MS::selectedModel())->toStdString();
    std::cerr << "[INFO] FaceTools::Action::ActionUpdateU3D::doAfterAction: U3D cached at '" << fpath << "'" << std::endl;
#endif
    return Event::CACHE;
}   // end doAfterAction
