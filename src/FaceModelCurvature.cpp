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

#include <FaceModelCurvature.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::FaceModelCurvature;
using FaceTools::FM;

std::unordered_map<const FM*, r3d::Curvature::Ptr> FaceModelCurvature::_metrics;
QReadWriteLock FaceModelCurvature::_lock;


FaceModelCurvature::RPtr FaceModelCurvature::rmetrics( const FM *fm)
{
    _lock.lockForRead();

    const r3d::Curvature *cm = _metrics.count(fm) > 0 ?  _metrics.at(fm).get() : nullptr;
    if ( !cm)
    {
        _lock.unlock();
        return nullptr;
    }   // end if

    return RPtr( cm, []( const r3d::Curvature*){ _lock.unlock();});
}   // end rmetrics


FaceModelCurvature::WPtr FaceModelCurvature::wmetrics( const FM *fm)
{
    _lock.lockForWrite();

    r3d::Curvature *cm = _metrics.count(fm) > 0 ?  _metrics.at(fm).get() : nullptr;
    if ( !cm)
    {
        _lock.unlock();
        return nullptr;
    }   // end if

    return WPtr( cm, []( r3d::Curvature*){ _lock.unlock(); });
}   // end wmetrics


void FaceModelCurvature::purge( const FM *fm)
{
    _lock.lockForWrite();
    _metrics.erase(fm);
    _lock.unlock();
}   // end purge


void FaceModelCurvature::add( const FM *fm)
{
    r3d::Curvature::Ptr cmap = r3d::Curvature::create( fm->mesh());  // Blocks
    _lock.lockForWrite();
    assert( _metrics.count(fm) == 0);
    _metrics[fm] = cmap;
    _lock.unlock();
}   // end add
