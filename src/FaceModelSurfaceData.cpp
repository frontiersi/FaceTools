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
using FaceTools::FaceModel;

std::shared_ptr<FaceModelSurfaceData> FaceModelSurfaceData::s_ptr(nullptr);  // static definition

// public static
FaceModelSurfaceData* FaceModelSurfaceData::get()
{
    if ( !s_ptr)
        s_ptr = std::shared_ptr<FaceModelSurfaceData>( new FaceModelSurfaceData, [](auto d){delete d;});
    return s_ptr.get();
}   // end get

// public static
SurfaceData::RPtr FaceModelSurfaceData::rdata( const FaceModel* fm)
{
    assert( get()->_data.count(fm) > 0);
    return get()->_data.at(fm)->readLock();
}   // end rdata


// public static
SurfaceData::WPtr FaceModelSurfaceData::wdata( const FaceModel* fm)
{
    assert( get()->_data.count(fm) > 0);
    return get()->_data.at(fm)->writeLock();
}   // end wdata


// private
FaceModelSurfaceData::~FaceModelSurfaceData()
{
    while ( !_data.empty())
    {
        const FaceModel* fm = _data.begin()->first;
        if ( !purge( fm, 1000)) // Wait 1 second to purge
        {
            if ( QThread::currentThread() != _data.at(fm)->thread())
            {
                std::cerr << "[WARNING] FaceTools::FaceModelSurfaceData::dtor: TERMINATING THREAD!" << std::endl;
                _data.at(fm)->thread()->terminate();
            }   // end if
            purge(fm);
        }   // end if
    }   // end while
}   // end dtor


// public
bool FaceModelSurfaceData::isAvailable( const FaceModel* fm) const
{
    return _data.count(fm) > 0 && !_data.at(fm)->working;
}   // end isAvailable


// public
void FaceModelSurfaceData::calculate( FaceModel* fm)
{
    if ( _data.count(fm) == 0)
    {
        SurfaceDataWorker* sdw = _data[fm] = new SurfaceDataWorker(fm);
        connect( sdw, &SurfaceDataWorker::onCalculated, this, &FaceModelSurfaceData::onCalculated);
        sdw->calculate();
    }   // end if
}   // end calculate


// public
bool FaceModelSurfaceData::purge( const FaceModel* fm, unsigned long wmsecs)
{
    if ( _data.count(fm) == 0)
        return true;

    // It's possible that _data.at(fm) is locked for writing - so wait until that thread is done if so.
    if ( !_data.at(fm)->lock.tryLockForWrite())
    {
        // Wait for thread to finish
        if ( !_data.at(fm)->thread()->wait(wmsecs))
        {
            std::cerr << "[WARNING] FaceTools::FaceModelSurfaceData::purge: Wait for thread to finish timed out!" << std::endl;
            return false;
        }   // end if
        if ( !_data.at(fm)->lock.tryLockForWrite())  // Try again - absolutely should work this time!
        {
            std::cerr << "[ERROR] FaceTools::FaceModelSurfaceData::purge: Waited for thread to finish, but couldn't acquire write lock!" << std::endl;
            return false;
        }   // end if
    }   // end if

    _data.at(fm)->lock.unlock();
    delete _data.at(fm);
    _data.erase(fm);
    return true;
}   // end purge
