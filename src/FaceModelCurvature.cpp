/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
using OMCM = RFeatures::ObjModelCurvatureMap;


std::unordered_map<const FM*, OMCM::Ptr> FaceModelCurvature::_metrics;
QReadWriteLock FaceModelCurvature::_lock;


FaceModelCurvature::RPtr FaceModelCurvature::rmetrics( const FM* fm)
{
    _lock.lockForRead();

    const OMCM* cm = _metrics.count(fm) > 0 ?  _metrics.at(fm).get() : nullptr;
    if ( !cm)
    {
        _lock.unlock();
        return nullptr;
    }   // end if

    return RPtr( cm, []( const OMCM*){ _lock.unlock(); });
}   // end rmetrics


FaceModelCurvature::WPtr FaceModelCurvature::wmetrics( const FM* fm)
{
    _lock.lockForWrite();

    OMCM* cm = _metrics.count(fm) > 0 ?  _metrics.at(fm).get() : nullptr;
    if ( !cm)
    {
        _lock.unlock();
        return nullptr;
    }   // end if

    return WPtr( cm, []( OMCM*){ _lock.unlock(); });
}   // end wmetrics


void FaceModelCurvature::purge( const FM* fm)
{
    _lock.lockForWrite();
    _metrics.erase(fm);
    _lock.unlock();
}   // end purge


void FaceModelCurvature::add( const FM* fm)
{
    assert( _metrics.count(fm) == 0);
    OMCM::Ptr omcm = OMCM::create( fm->model(), fm->manifolds());  // Blocks
    _lock.lockForWrite();
    _metrics[fm] = omcm;
    _lock.unlock();
}   // end add
