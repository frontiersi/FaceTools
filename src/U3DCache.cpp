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

#include <U3DCache.h>
#include <Vis/FaceView.h>
#include <r3dio/U3DExporter.h>
#include <r3dvis/VtkTools.h>
#include <QTemporaryFile>
#include <QTools/QImageTools.h>
#include <cassert>
using FaceTools::U3DCache;
using FaceTools::FM;
using FaceTools::Vis::FV;

// static definitions
QTemporaryDir U3DCache::_tmpdir;
QReadWriteLock U3DCache::_rwLock;
std::unordered_set<const FM*> U3DCache::_cache;


U3DCache::Filepath U3DCache::u3dfilepath( const FM &fm)
{
    QString fname;
    if ( !_rwLock.tryLockForRead())
        return nullptr;
    if ( _cache.count(&fm) > 0)
        fname = _makeFilePath(fm);
    return Filepath( new QString( fname), []( QString* s){ delete s; _rwLock.unlock();});
}   // end u3dfilepath


bool U3DCache::isAvailable()
{
    bool avail = false;
    if ( _rwLock.tryLockForRead())
    {
        avail = r3dio::U3DExporter::isAvailable();
        _rwLock.unlock();
    }   // end if
    return avail;
}   // end isAvailable


namespace {
std::string exportU3D( const r3d::Mesh &mesh, const QString &fname,
                       const r3d::Colour &ems=r3d::Colour::white())
{
    std::string errmsg = "";
    QTemporaryFile tfile( QDir::tempPath() + "/XXXXXX.u3d");
    if ( tfile.open())
    {
        r3dio::U3DExporter xptr( true/*false for debug*/, true/*media9*/, ems);
        if ( xptr.save( mesh, tfile.fileName().toLocal8Bit().toStdString()))
        {
            if ( !QFile::copy( tfile.fileName(), fname))
                errmsg = "Unable to copy generated U3D to required location!";
            QFile::remove( tfile.fileName());
        }   // end if
        else
            errmsg = "Unable to save to U3D format!";
    }   // end if
    else
        errmsg = "Couldn't open temporary file for U3D export!";
    return errmsg;
}   // end exportU3D
}   // end namespace


bool U3DCache::refresh( const FM &fm)
{
    fm.lockForRead();
    r3d::Mesh::Ptr mesh = fm.mesh().deepCopy();
    fm.unlock();
    assert( mesh->hasSequentialVertexIds());

    r3d::Colour ems = r3d::Colour::white();
    if ( !mesh->hasMaterials())
    {
        const QColor bc = FV::BASECOL;
        ems = r3d::Colour( bc.red(), bc.green(), bc.blue());
        /*
        // Add a flat texture since none present
        static const QImage timg(":/textures/BASE_BLUE");
        if ( timg.isNull())
            std::cerr << "timg is null! Cannot add pseudo texture!" << std::endl;
        else
        {
            const cv::Mat mat = QTools::copyQImage2OpenCV( timg);
            const int mid = mesh->addMaterial( mat);
            static const Vec2f uvs[3] = {Vec2f(1,0), Vec2f(1,1), Vec2f(0,0)};
            for ( int fid : mesh->faces())
                mesh->setOrderedFaceUVs( mid, fid, uvs);
        }   // end else
        */
    }   // end if

    const std::string errmsg = exportU3D( *mesh, _makeFilePath(fm), ems);
    if ( errmsg.empty())
    {
        _rwLock.lockForWrite();
        _cache.insert(&fm);
        _rwLock.unlock();
    }   // end if
    else
        std::cerr << "[ERROR] FaceTools::U3DCache::refresh: " << errmsg << std::endl;
    return errmsg.empty();
}   // end refresh


r3d::Mesh::Ptr U3DCache::makeU3D( const FV *fv, const QString &u3dfile)
{
    const FM *fm = fv->data();
    fm->lockForRead();
    r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();
    fm->unlock();
    mesh->removeAllMaterials();
    r3dvis::mapActiveScalarsToMesh( fv->actor(), *mesh);
    const std::string errmsg = exportU3D( *mesh, u3dfile);
    if ( !errmsg.empty())
    {
        std::cerr << "[ERROR] FaceTools::U3DCache::makeU3D: " << errmsg << std::endl;
        mesh = nullptr;
    }   // end if
    return mesh;
}   // end makeU3D


void U3DCache::purge( const FM &fm)
{
    _rwLock.lockForWrite();
    if ( _cache.count(&fm) > 0)
    {
        QFile::remove( _makeFilePath(fm));
        _cache.erase(&fm);
    }   // end if
    _rwLock.unlock();
}   // end purge


QString U3DCache::_makeFilePath( const FM &fm)
{
    QString fname;
    QTextStream os(&fname);
    os << "obj_" << Qt::hex << &fm << ".u3d";
    fname = _tmpdir.filePath( fname);   // The filename to save to
    return fname;
}   // end _makeFilePath
