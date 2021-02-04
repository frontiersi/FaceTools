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

#include <FaceTools/FaceModelSymmetryStore.h>
#include <FaceTools/FaceModel.h>
#include <cassert>
using FaceTools::FaceModelSymmetryStore;
using FaceTools::FaceModelSymmetry;
using FaceTools::FM;

std::unordered_map<const FM*, FaceModelSymmetry::Ptr> FaceModelSymmetryStore::_vtxSymm;
QReadWriteLock FaceModelSymmetryStore::_lock;

using RPtr = std::shared_ptr<const FaceModelSymmetry>;
RPtr FaceModelSymmetryStore::vals( const FM *fm)
{
    if ( !_lock.tryLockForRead())
        return nullptr;
    const FaceModelSymmetry *vvals = _vtxSymm.count(fm) > 0 ? _vtxSymm.at(fm).get() : nullptr;
    if ( !vvals)
    {
        _lock.unlock();
        return nullptr;
    }   // end if
    return RPtr( vvals, []( const FaceModelSymmetry*){ _lock.unlock();});
}   // end vals


bool FaceModelSymmetryStore::isMapped( const FM *fm)
{
    assert( fm);
    bool gotvals = false;
    if ( _lock.tryLockForRead())
    {
        gotvals = _vtxSymm.count(fm) > 0;
        _lock.unlock(); 
    }   // end if
    return gotvals;
}   // end isMapped


void FaceModelSymmetryStore::purge( const FM *fm)
{
    _lock.lockForWrite();
    _vtxSymm.erase(fm);
    _lock.unlock();
}   // end purge


void FaceModelSymmetryStore::add( const FM *fm)
{
    assert( _vtxSymm.count(fm) == 0);
    _lock.lockForWrite();
    _vtxSymm[fm] = FaceModelSymmetry::create(fm);
    _lock.unlock();
}   // end add
