/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceControl.h>
#include <FaceModel.h>
#include <QThread>
#include <algorithm>
using FaceTools::Action::ActionMapSurfaceData;
using FaceTools::Action::ChangeEventSet;
using FaceTools::FaceModelSurfaceData;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;


// public
ActionMapSurfaceData::ActionMapSurfaceData()
{
    addPurgeOn( GEOMETRY_CHANGE);
    addProcessOn( GEOMETRY_CHANGE);
    connect( FaceModelSurfaceData::get(), &FaceModelSurfaceData::onCalculated, this, &ActionMapSurfaceData::doOnCalculated);
}   // end ctor


bool ActionMapSurfaceData::doAction( FaceControlSet& rset, const QPoint&)
{
    for ( FaceModel* fm : rset.models())
    {
        if ( !FaceModelSurfaceData::get()->isAvailable(fm))
        {
            fm->lockForRead();
            FaceModelSurfaceData::get()->calculate(fm);
        }   // end if
    }   // end for
    return true;
}   // end doAction


void ActionMapSurfaceData::purge( const FaceModel* fm)
{
    FaceModelSurfaceData::get()->purge(fm);
}   // end purge


void ActionMapSurfaceData::doOnCalculated( const FaceModel* fm)
{
    fm->unlock();
    ChangeEventSet cset;
    cset.insert( SURFACE_DATA_CHANGE);
    emit reportFinished( cset, fm->faceControls(), true);
}   // end doOnCalculated
