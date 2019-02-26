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

#include <U3DCache.h>
#include <FaceModel.h>
#include <U3DExporter.h>
#include <FaceViewSet.h>
#include <Transformer.h>
#include <QtDebug>
#include <algorithm>
#include <cassert>
using FaceTools::Action::U3DCache;
using FaceTools::FM;
using FaceTools::FMS;


std::shared_ptr<U3DCache> U3DCache::_singleton;
QReadWriteLock U3DCache::_lock( QReadWriteLock::Recursive);
QTemporaryDir U3DCache::_tmpdir;
FMS U3DCache::_cache;
QThread U3DCache::_wthread;
bool U3DCache::_media9(false);
FM* U3DCache::_fm(nullptr);


// private static
std::shared_ptr<U3DCache> U3DCache::get()
{
    if ( !_singleton)
    {
        //_tmpdir.setAutoRemove(false); // Uncomment for debug
        _singleton = std::shared_ptr<U3DCache>( new U3DCache, [](U3DCache* x){ delete x;});
    }   // end if
    return _singleton;
}   // end get


QString U3DCache::lock( const FM* fm)
{
    QString fname;
    _lock.lockForRead();
    if ( _cache.count(const_cast<FM*>(fm)) > 0)
        fname = u3dfile(fm);
    else
        _lock.unlock();
    return fname;
}   // end lock


void U3DCache::release() { _lock.unlock();}


void U3DCache::refresh( const FM* fm, bool med9)
{
    _fm = const_cast<FM*>(fm);
    _media9 = med9;
    get()->start();
}   // end refresh


QString U3DCache::u3dfile( const FM* fm)
{
    QString fname;
    QTextStream os(&fname);
    os << "obj_" << hex << fm << ".u3d";
    fname = _tmpdir.filePath( fname);   // The filename to save to
    return fname;
}   // end u3dfile


void U3DCache::run()
{
    _lock.lockForWrite();
    qInfo( "Making local copy of model");
    RFeatures::ObjModel::Ptr model = RFeatures::ObjModel::copy( _fm->info()->cmodel(), true/*shared materials*/);

    // Ensure that the model is in standard position:
    qInfo( "Transforming model to standard position");
    const cv::Vec3f centre = _fm->centre();
    const RFeatures::Orientation on = _fm->orientation();
    cv::Matx44d tmat = RFeatures::toStandardPosition( on.nvec(), on.uvec(), centre);
    RFeatures::Transformer transformer( tmat);
    transformer.transform( model);

    const QString fname = u3dfile(_fm);
    qInfo() << "Exporting to U3D format at" << fname;
    RModelIO::U3DExporter xptr( true, _media9);
    if ( xptr.save( model.get(), fname.toStdString()))
    {
        _cache.insert(_fm);
        qInfo( "Finished save to U3D format");
    }   // end if
    else
        qWarning( "Unable to save to U3D format!");

    _fm = nullptr;
    _lock.unlock();
}   // end run


void U3DCache::purge( const FM* fm) { get()->_purge( fm);}

void U3DCache::_purge( const FM* cfm)
{
    _lock.lockForWrite();
    FM* fm = const_cast<FM*>(cfm);
    if ( _cache.count(fm) > 0)
    {
        const QString fname = u3dfile(fm);
        QFile::remove( fname);
        _cache.erase(fm);
    }   // end if
    _lock.unlock();
}   // end _purge
