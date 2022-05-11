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

#include <FaceTools/FaceModelDeltaStore.h>
#include <FaceTools/FaceModel.h>
#include <cassert>
using FaceTools::FaceModelDeltaStore;
using FMD = FaceTools::FaceModelDelta;
using FaceTools::FM;


std::unordered_map<const FM*, std::unordered_map<const FM*, FMD::Ptr> > FaceModelDeltaStore::_t2s;
std::unordered_map<const FM*, std::unordered_map<const FM*, FMD::Ptr> > FaceModelDeltaStore::_s2t;
QReadWriteLock FaceModelDeltaStore::_lock;


bool FaceModelDeltaStore::_has( const FM *tgt, const FM *src)
{
    return (_t2s.count(tgt) > 0) && (_t2s.at(tgt).count(src) > 0);
}   // end _has


using RPtr = std::shared_ptr<const FMD>;
RPtr FaceModelDeltaStore::vals( const FM *tgt, const FM *src)
{
    if ( !_lock.tryLockForRead())
        return nullptr;
    if ( !_has(tgt, src))
    {
        _lock.unlock();
        return nullptr;
    }   // end if
    return RPtr( _t2s.at(tgt).at(src).get(), []( const FMD*){ _lock.unlock();});
}   // end vals


void FaceModelDeltaStore::add( const FM *tgt, const FM *src)
{
    _lock.lockForWrite();
    FMD::Ptr fmd = FMD::create( tgt, src);
    assert(fmd);
    _t2s[tgt][src] = fmd;
    _s2t[src][tgt] = fmd;
    _lock.unlock();
}   // end add


bool FaceModelDeltaStore::has( const FM *tgt, const FM *src)
{
    if ( !_lock.tryLockForRead())
        return false;
    const bool h = _has( tgt, src);
    _lock.unlock();
    return h;
}   // end has


void FaceModelDeltaStore::purge( const FM *fm)
{
    _lock.lockForWrite();
    // All the targets using fm as a source need purging
    if ( _s2t.count(fm) > 0)
    {
        for ( const auto& p : _s2t.at(fm))
        {
            const FM *tgt = p.first;
            if ( _t2s.count(tgt) > 0)
            {
                _t2s.at(tgt).erase(fm);
                if ( _t2s.at(tgt).empty())
                    _t2s.erase(tgt);
            }   // end if
        }   // end for
        _s2t.erase(fm);
    }   // end if
    _t2s.erase(fm);
    _lock.unlock();
}   // end purge
