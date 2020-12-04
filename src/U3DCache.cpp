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

#include <U3DCache.h>
#include <FaceModel.h>
#include <r3dio/U3DExporter.h>
#include <QTemporaryFile>
#include <QTools/QImageTools.h>
#include <cassert>
using FaceTools::U3DCache;
using FaceTools::FM;

QTemporaryDir U3DCache::_tmpdir;
QReadWriteLock U3DCache::_cacheLock;
std::unordered_set<const FM*> U3DCache::_cache;


U3DCache::Filepath U3DCache::u3dfilepath( const FM* fm)
{
    QString fname;
    _cacheLock.lockForRead();
    if ( _cache.count(fm) > 0)
        fname = makeFilePath(fm);
    return Filepath( new QString( fname), []( QString* s){ delete s; _cacheLock.unlock();});
}   // end lock


bool U3DCache::isAvailable()
{
    bool avail = false;
    if ( _cacheLock.tryLockForRead())
    {
        avail = r3dio::U3DExporter::isAvailable();
        _cacheLock.unlock();
    }   // end if
    return avail;
}   // end isAvailable


bool U3DCache::refresh( const FM* fm, bool med9)
{
    // Copy out model
    fm->lockForRead();
    r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();
    fm->unlock();

    // Add a texture if none present
    float ambv = 1.0f;
    if ( !mesh->hasMaterials())
    {
        ambv = 0.3f;    // Allow flat textured models to show up with less ambient light reflected by their material
        const cv::Mat mat = QTools::copyQImage2OpenCV( QImage(":/imgs/BASE_BLUE"));
        mesh->addMaterial( mat);
        static const Vec2f uvs[3] = {Vec2f(1,0), Vec2f(1,1), Vec2f(0,0)};
        for ( int fid : mesh->faces())
            mesh->setOrderedFaceUVs( 0, fid, uvs);
    }   // end if

    bool refreshed = false;
    _cacheLock.lockForWrite();
    QTemporaryFile tfile( QDir::tempPath() + "/XXXXXX.u3d");
    if ( tfile.open())
    {
        r3dio::U3DExporter xptr( true/*set true to delete IDTF file on destruction*/, med9, ambv);
        //std::cerr << QString( "Exporting U3D model to '%1'").arg( tfile.fileName()).toStdString() << std::endl;
        if ( xptr.save( *mesh, tfile.fileName().toLocal8Bit().toStdString()))
        {
            // Copy U3D model exported to the temporary location to the cache location
            const QString cacheFileName = makeFilePath(fm);
            QFile::copy( tfile.fileName(), cacheFileName);
            _cache.insert(fm);
            QFile::remove( tfile.fileName());
            refreshed = true;
        }   // end if
        else
            std::cerr << "[ERROR] FaceTools::U3DCache::refresh: Unable to save to U3D format!" << std::endl;
    }   // end if
    else
        std::cerr << "[ERROR] FaceTools::U3DCache::refresh: Couldn't open temporary file for exporting U3D!" << std::endl;
    _cacheLock.unlock();

    return refreshed;
}   // end refresh


void U3DCache::purge( const FM* fm)
{
    _cacheLock.lockForWrite();
    if ( _cache.count(fm) > 0)
    {
        QFile::remove( makeFilePath(fm));
        _cache.erase(fm);
    }   // end if
    _cacheLock.unlock();
}   // end purge


// private
QString U3DCache::makeFilePath( const FM* fm)
{
    QString fname;
    QTextStream os(&fname);
    os << "obj_" << Qt::hex << fm << ".u3d";
    fname = _tmpdir.filePath( fname);   // The filename to save to
    return fname;
}   // end makeFilePath
