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

#include <FaceModelCurvatureStore.h>
#include <cassert>
using FaceTools::FaceModelCurvatureStore;
using FMC = FaceTools::FaceModelCurvature;
using FaceTools::FM;

std::unordered_map<const FM*, FMC::Ptr> FaceModelCurvatureStore::_metrics;
QReadWriteLock FaceModelCurvatureStore::_lock;


FaceModelCurvatureStore::RPtr FaceModelCurvatureStore::rvals( const FM &fm)
{
    if ( !_lock.tryLockForRead())
        return nullptr;
    const FMC *fmc = _metrics.count(&fm) > 0 ?  _metrics.at(&fm).get() : nullptr;
    if ( !fmc)
    {
        _lock.unlock();
        return nullptr;
    }   // end if
    return RPtr( fmc, []( const FMC*){ _lock.unlock();});
}   // end rvals


FaceModelCurvatureStore::WPtr FaceModelCurvatureStore::wvals( const FM &fm)
{
    _lock.lockForWrite();
    FMC *fmc = _metrics.count(&fm) > 0 ?  _metrics.at(&fm).get() : nullptr;
    if ( !fmc)
    {
        _lock.unlock();
        return nullptr;
    }   // end if
    return WPtr( fmc, []( const FMC*){ _lock.unlock();});
}   // end wvals


void FaceModelCurvatureStore::purge( const FM &fm)
{
    _lock.lockForWrite();
    _metrics.erase(&fm);
    _lock.unlock();
}   // end purge


void FaceModelCurvatureStore::add( const FM &fm)
{
    FMC::Ptr fmc = FMC::create( fm.mesh());  // Blocks
    _lock.lockForWrite();
    assert( _metrics.count(&fm) == 0);
    _metrics[&fm] = fmc;
    _lock.unlock();
}   // end add
