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

#include <FaceModelSurfaceData.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::FaceModelSurfaceData;
using FaceTools::SurfaceDataWorker;
using FaceTools::SurfaceData;
using FaceTools::FM;

std::shared_ptr<FaceModelSurfaceData> FaceModelSurfaceData::s_ptr(nullptr);  // static definition

// public static
FaceModelSurfaceData* FaceModelSurfaceData::get()
{
    if ( !s_ptr)
        s_ptr = std::shared_ptr<FaceModelSurfaceData>( new FaceModelSurfaceData, [](FaceModelSurfaceData* d){delete d;});
    return s_ptr.get();
}   // end get


// private static
std::unordered_map<const FM*, SurfaceDataWorker*>& FaceModelSurfaceData::data()
{
    assert( s_ptr);
    return s_ptr->_data;
}   // end data


// public static
SurfaceData::RPtr FaceModelSurfaceData::rdata( const FM* fm)
{
    assert( data().count(fm) > 0);
    return data().at(fm)->readLock();
}   // end rdata


// public static
SurfaceData::WPtr FaceModelSurfaceData::wdata( const FM* fm)
{
    assert( data().count(fm) > 0);
    return data().at(fm)->writeLock();
}   // end wdata


// private
FaceModelSurfaceData::~FaceModelSurfaceData()
{
    while ( !_data.empty())
    {
        const FM* fm = _data.begin()->first;
        if ( !purge( fm, 1000)) // Wait 1 second to purge
        {
            qWarning( "TERMINATING THREAD!");
            _data.at(fm)->terminate();
            purge(fm);
        }   // end if
    }   // end while
}   // end dtor


// public static
bool FaceModelSurfaceData::isAvailable( const FM* fm)
{
    return data().count(fm) > 0 && !data().at(fm)->working;
}   // end isAvailable


// public static
void FaceModelSurfaceData::calculate( FM* fm)
{
    if ( data().count(fm) == 0)
    {
        SurfaceDataWorker* sdw = data()[fm] = new SurfaceDataWorker(fm);
        connect( sdw, &SurfaceDataWorker::onCalculated, get(), &FaceModelSurfaceData::onCalculated);
        sdw->start();   // Returns immediately
    }   // end if
}   // end calculate


// public static
bool FaceModelSurfaceData::purge( const FM* fm, unsigned long wmsecs)
{
    if ( data().count(fm) == 0)
        return true;

    // It's possible that _data.at(fm) is locked for writing - so wait until that thread is done if so.
    if ( !data().at(fm)->lock.tryLockForWrite())
    {
        // Wait for thread to finish
        if ( !data().at(fm)->wait(wmsecs))
        {
            qWarning( "Wait for thread to finish timed out!");
            return false;
        }   // end if
        if ( !data().at(fm)->lock.tryLockForWrite())  // Try again - absolutely should work this time!
        {
            qWarning( "Waited for thread to finish, but couldn't acquire write lock!");
            return false;
        }   // end if
    }   // end if

    data().at(fm)->lock.unlock();
    delete data().at(fm);
    data().erase(fm);
    return true;
}   // end purge
