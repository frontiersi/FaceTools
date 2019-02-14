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

#include <ActionMapSurfaceData.h>
#include <FaceModelSurfaceData.h>
#include <FaceModel.h>
#include <QThread>
#include <algorithm>
using FaceTools::Action::ActionMapSurfaceData;
using FaceTools::Action::EventSet;
using FaceTools::FaceModelSurfaceData;
using FaceTools::FaceModel;
using FaceTools::FVS;


// public
ActionMapSurfaceData::ActionMapSurfaceData()
{
    setPurgeOnEvent( GEOMETRY_CHANGE);
    setRespondToEvent( GEOMETRY_CHANGE);
    connect( FaceModelSurfaceData::get(), &FaceModelSurfaceData::onCalculated, this, &ActionMapSurfaceData::doOnCalculated);
}   // end ctor


bool ActionMapSurfaceData::doAction( FVS& rset, const QPoint&)
{
    for ( FaceModel* fm : rset.models())
    {
        if ( !FaceModelSurfaceData::isAvailable(fm))
        {
            fm->lockForRead();
            FaceModelSurfaceData::calculate(fm);
        }   // end if
    }   // end for
    return true;
}   // end doAction


void ActionMapSurfaceData::purge( const FaceModel* fm)
{
    FaceModelSurfaceData::purge(fm);
}   // end purge


void ActionMapSurfaceData::doOnCalculated( const FaceModel* fm)
{
    fm->unlock();
    EventSet cset;
    cset.insert( SURFACE_DATA_CHANGE);
    emit reportFinished( cset, fm->fvs(), true);
}   // end doOnCalculated
