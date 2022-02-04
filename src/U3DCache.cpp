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
#include <boost/filesystem.hpp>
using FaceTools::U3DCache;
using FaceTools::FM;
using FaceTools::Vis::FV;
namespace BFS = boost::filesystem;

// static definitions
QTemporaryDir U3DCache::_tmpdir;
QReadWriteLock U3DCache::_rwLock;
std::unordered_map<const FM*, QString> U3DCache::_cache;


U3DCache::Filepath U3DCache::u3dfilepath( const FM &fm)
{
    QString fname;
    if ( !_rwLock.tryLockForRead())
        return nullptr;
    if ( _cache.count(&fm) > 0)
        fname = _cache.at(&fm);
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


bool U3DCache::_exportU3D( const r3d::Mesh &mesh, const QString &savepath, const rimg::Colour &ems)
{
    std::string errmsg = "";
    r3dio::U3DExporter xptr( true/*delete on destroy*/, true/*media9*/, ems);
    if ( !xptr.save( mesh, savepath.toStdString()))
    {
        errmsg = "Unable to save to U3D format!";
        std::cerr << "[ERROR] FaceTools::U3DCache::_exportU3D: " << errmsg << std::endl;
    }   // end if
    return errmsg.empty();
}   // end _exportU3D


bool U3DCache::refresh( const FM &fm)
{
    fm.lockForRead();
    r3d::Mesh::Ptr mesh = fm.mesh().deepCopy();
    fm.unlock();
    assert( mesh->hasSequentialVertexIds());

    rimg::Colour ems = rimg::Colour::white();
    if ( !mesh->hasMaterials())
    {
        const QColor bc = FV::BASECOL;
        ems = rimg::Colour( bc.red(), bc.green(), bc.blue());
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

    const std::string upath = BFS::unique_path( "%%%%-%%%%-%%%%-%%%%.u3d").string();
    const QString fname = QString("%1_%2").arg(fm.subjectId()).arg( QString::fromStdString(upath));
    const QString savepath = _tmpdir.filePath( fname);
    const bool okay = _exportU3D( *mesh, savepath, ems);

    // Update the reference to the newly cached U3D and remove the old U3D.
    if ( okay)
    {
        _rwLock.lockForWrite();
        if ( _cache.count(&fm) > 0)
            QFile::remove( _cache.at(&fm));
        _cache[&fm] = savepath;
        _rwLock.unlock();
    }   // end if

    return okay;
}   // end refresh


r3d::Mesh::Ptr U3DCache::makeColourMappedU3D( const FV *fv, const QString &u3dfile)
{
    const FM *fm = fv->data();
    fm->lockForRead();
    r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();
    fm->unlock();
    mesh->removeAllMaterials();
    r3dvis::mapActiveScalarsToMesh( fv->actor(), *mesh);
    const rimg::Colour ems = rimg::Colour::white();
    if ( !_exportU3D( *mesh, u3dfile, ems))
        mesh = nullptr;
    return mesh;
}   // end makeColourMappedU3D


void U3DCache::purge( const FM &fm)
{
    _rwLock.lockForWrite();
    if ( _cache.count(&fm) > 0)
    {
        QFile::remove( _cache.at(&fm));
        _cache.erase(&fm);
    }   // end if
    _rwLock.unlock();
}   // end purge

