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

#include <MaskRegistration.h>
#include <FileIO/FaceModelXMLFileHandler.h>
#include <FileIO/FaceModelManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <rNonRigid.h>
#include <QFileInfo>
#include <QThread>
#include <r3d/ProcrustesSuperimposition.h>
#include <r3d/Bounds.h>
#include <boost/filesystem/path.hpp>
#include <boost/functional/hash.hpp>
//#include <thread>
using FaceTools::MaskRegistration;
using FaceTools::FaceSide;
using FMM = FaceTools::FileIO::FaceModelManager;


MaskRegistration::MaskData MaskRegistration::s_mask;
QReadWriteLock MaskRegistration::s_lock;


MaskRegistration::MaskData::MaskData() : mask(nullptr) {}


namespace {

void setBarycentricLandmarkPositions( std::unordered_map<int, std::pair<int, r3d::Vec3f> >& tset,
                                      const std::unordered_map<int, r3d::Vec3f> &sset,
                                      const r3d::KDTree &kdt)
{
    const r3d::Mesh &mesh = kdt.mesh();
    int fid = -1;
    const r3d::SurfacePointFinder spfinder( mesh);
    for ( const auto &p : sset)
    {
        r3d::Vec3f fpos = p.second;  // Landmark position (ensure in surface)

        int svidx = kdt.find( fpos);
        spfinder.find( fpos, svidx, fid, fpos);
        if ( fid < 0)   // If fid < 0, then fpos == vtx(svidx) so choose any face attached to svidx
            fid = *mesh.faces(svidx).begin();

        // Key by landmark ID, the barycentric coordinates of the landmark with respect to the mesh face it was found it.
        tset[p.first] = std::pair<int, r3d::Vec3f>( fid, mesh.toBarycentric( fid, fpos));
    }   // end for
}   // end setBarycentricLandmarkPositions


size_t createHash( const r3d::Mesh &mesh)
{
    assert( mesh.hasSequentialIds());
    static const size_t HASH_NDP = 4;
    size_t h = 0;

    // Hash together the vertex positions
    const size_t N = mesh.numVtxs();
    for ( size_t i = 0; i < N; ++i)
        r3d::hash( mesh.uvtx(int(i)), HASH_NDP, h);

    // Hash the topology
    const size_t M = mesh.numFaces();
    for ( size_t i = 0; i < M; ++i)
    {
        const int* fvidxs = mesh.fvidxs(int(i));
        boost::hash_combine( h, fvidxs[0]);
        boost::hash_combine( h, fvidxs[1]);
        boost::hash_combine( h, fvidxs[2]);
    }   // end for

    return h;
}   // end createHash


void binPointIndices( MaskRegistration::MaskData &md, float y, int vidx, int ovidx)
{
    if ( y >= 0)
    {
        md.q0.insert(vidx);
        md.q1.insert(ovidx);
    }   // end if
    if ( y <= 0)
    {
        md.q2.insert(ovidx);
        md.q3.insert(vidx);
    }   // end if
}   // end binPointIndices

}   // end namespace


void MaskRegistration::unsetMask()
{
    if ( s_mask.mask)
    {
        delete s_mask.mask;
        s_mask.mask = nullptr;
        s_mask.path = "";
        s_mask.hash = 0;
    }   // end if
}   // end unsetMask



bool MaskRegistration::setMask( const QString &mpath)
{
    const QString abspath = QFileInfo( mpath).absoluteFilePath();
    if ( s_mask.path == abspath)
        return true;

    unsetMask();

    if ( !FMM::canRead( mpath) || !FMM::isPreferredFileFormat( mpath))
        return false;

    // Load meta data - fail if no landmarks present.
    PTree ptree;
    QTemporaryDir *tdir = new QTemporaryDir;
    FileIO::readMeta( mpath, *tdir, ptree);

    FM *fm = new FM;
    double fversion;
    QString meshfname, unused;
    if ( !FileIO::importMetaData( *fm, ptree, fversion, meshfname, unused))
    {
#ifndef NDEBUG
        std::cerr << "[WARNING] FaceTools::MaskRegistration::setMask: Invalid metadata!" << std::endl;
#endif
        delete fm;
        return false;
    }   // end if

    if ( !fm->hasLandmarks())
    {
#ifndef NDEBUG
        std::cerr << "[WARNING] FaceTools::MaskRegistration::setMask: No landmarks in mask!" << std::endl;
#endif
        delete fm;
        return false;
    }   // end if

    //std::cout << "Loading anthropomorphic mask for surface registration..." << std::endl;
    QThread *thread = QThread::create(
        [abspath, meshfname, tdir, fm]()
        {
            QString unused;
            const QString err = FileIO::loadData( *fm, *tdir, meshfname, unused);
            if ( err.isEmpty())
            {
                s_lock.lockForWrite();
                s_mask.mask = fm;
                s_mask.path = abspath;
                s_mask.hash = createHash( fm->mesh());

                // Note that there's an opportunity here to have a mask store several different
                // sets of landmarks (perhaps derived from different assessors of whatever).
                const Landmark::LandmarkSet& lmset = fm->currentLandmarks();
                setBarycentricLandmarkPositions( s_mask.lmksL, lmset.lateral( LEFT), fm->kdtree());
                setBarycentricLandmarkPositions( s_mask.lmksM, lmset.lateral( MID), fm->kdtree());
                setBarycentricLandmarkPositions( s_mask.lmksR, lmset.lateral( RIGHT), fm->kdtree());

                // Set the laterally opposite vertex IDs:
                for ( int vidx : fm->mesh().vtxIds())
                {
                    if ( s_mask.oppVtxs.count(vidx) > 0)
                        continue;

                    // Reflect the vertex through the medial plane and find the closest opposite vertex.
                    // It is assumed that the medial plane lies at X=0 and that the mesh is upright and laterally symmetric.
                    const Vec3f &p = fm->mesh().vtx(vidx);
                    const int ovidx = fm->kdtree().find( Vec3f( -p[0], p[1], p[2]));
                    s_mask.oppVtxs[ovidx] = vidx;
                    s_mask.oppVtxs[vidx] = ovidx;
                    if ( ovidx == vidx)
                       s_mask.medialVtxs.insert(vidx);
                    // Partitioning of the vertices into the four quadrants is not mutually exclusive.
                    if ( p[0] >= 0)
                        binPointIndices( s_mask, p[1], vidx, ovidx);
                    if ( p[0] <= 0)
                        binPointIndices( s_mask, p[1], ovidx, vidx);
                }   // end for

                // Get the centre and height from just the medial vertices
                const r3d::Bounds bnds( fm->mesh(), Mat4f::Identity(), &s_mask.medialVtxs);
                s_mask.centre = bnds.centre();
                s_mask.radius = bnds.diagonal() / 2;
                s_lock.unlock();
            }   // end if
            else
            {
                std::cerr << "Failed to load mask data!" << std::endl;
                delete fm;
            }   // end else
            delete tdir;
        });
    QObject::connect( thread, &QThread::finished, [thread](){ thread->deleteLater();});
    thread->start();

    return true;
}   // end setMask


bool MaskRegistration::maskLoaded()
{
    if ( !s_lock.tryLockForRead())
        return false;
    const bool loaded = s_mask.mask != nullptr;
    s_lock.unlock();
    return loaded;
}   // end maskLoaded


QString MaskRegistration::maskPath()
{
    QString mpath;
    s_lock.lockForRead();
    mpath = s_mask.path;
    s_lock.unlock();
    return mpath;
}   // end maskPath


size_t MaskRegistration::maskHash()
{
    size_t h = 0;
    s_lock.lockForRead();
    h = s_mask.hash;
    s_lock.unlock();
    return h;
}   // end maskHash


r3d::Vec3f MaskRegistration::maskLandmarkPosition( const r3d::Mesh &msk, int lmid, FaceSide lat)
{
    const MaskPtr mdata = maskData();
    const std::unordered_map<int, std::pair<int, r3d::Vec3f> > *lmks = &mdata->lmksM; // MID default
    if ( lat == LEFT)
        lmks = &mdata->lmksL;
    else if ( lat == RIGHT)
        lmks = &mdata->lmksR;
    const std::pair<int, r3d::Vec3f> &bcds = lmks->at(lmid);
    return msk.fromBarycentric( bcds.first, bcds.second);
}   // end maskLandmarkPosition


std::shared_ptr<const MaskRegistration::MaskData> MaskRegistration::maskData()
{
    s_lock.lockForRead();
    return std::shared_ptr<const MaskData>( &s_mask, []( const MaskData*){ s_lock.unlock();});
}   // end maskData


r3d::Mesh::Ptr MaskRegistration::registerMask( const r3d::KDTree &kdt)
{
    static const std::string ISTR = " FaceTools::Action::MaskRegistration::registerMask: ";
    assert( maskLoaded());
    if ( !maskLoaded())
    {
        std::cerr << "[ERROR]" << ISTR << "Mask not loaded!" << std::endl;
        return nullptr;
    }   // end if

    const MaskPtr mdata = maskData();
    rNonRigid::Mesh flt;
    const r3d::Mesh &mask = mdata->mask->mesh();
    flt.features = mask.toFeatures( true/*use transformed*/);
    flt.topology = mask.toFaces();  // NB topology not needed for RigidRegistration

    rNonRigid::Mesh tgt;
    tgt.features = kdt.mesh().toFeatures( true/*use transformed*/);

    // Start with a 70% size mask since this empirically works better at fitting the
    // faces of babies and children without diminishing the ability to fit adult faces.
    Mat4f T = Mat4f::Identity() * 0.7f;
    T = rNonRigid::RigidRegistration( 20, 3, 0.9f, true, 4.0f, true, 10, true)( flt, tgt, T);
    const float minScale = std::min( T(0,0), std::min(T(1,1), T(2,2)));
    if ( minScale < 0.1f)
    {
        std::cerr << "[WARNING]" << ISTR << "Mask scaled to be too small!" << std::endl;
        return nullptr;
    }   // end if

#ifndef NDEBUG
    // Create mask to check that scaling didn't doesn't reduce the mask size too much.
    // If some vertex positions are too close after scaling, converting to r3d::Mesh
    // merges those points which is why checking for a count mismatch works.
    r3d::Mesh::Ptr tmask = r3d::Mesh::fromVertices( flt.features.leftCols(3));
    assert( flt.features.rows() == (long)tmask->numVtxs());
#endif

    // For the non-rigid registration, use a target face having vertices only a
    // little larger than the region covered by the rigidly registered mask.
    const Vec3f centre = r3d::transform( T, mdata->centre);
    const float radius = mdata->radius * T(1,1) * 1.15f;
    std::vector<std::pair<size_t, float> > vpts;
    kdt.findr( centre, radius*radius, vpts);
    rNonRigid::Mesh tgt2( vpts.size(), tgt.features.cols());
    int j = 0;
    for ( const std::pair<size_t, float> &p : vpts)
        tgt2.features.row(j++) = tgt.features.row(p.first);

    rNonRigid::NonRigidRegistration( 80, 3, 0.9f, true, 10.0f, true, 10, 50, 1.6f, 80, 1, 80, 1)( flt, tgt2);

    r3d::Mesh::Ptr cmask = r3d::Mesh::fromVertices( flt.features.leftCols(3)); // Make the final mask
    if ( flt.features.rows() != (long)cmask->numVtxs())
    {
        std::cerr << "[WARNING]" << ISTR << "Non-rigid registration merged vertices!" << std::endl;
        std::cerr << "\t# features = " << flt.features.rows() << std::endl;
        std::cerr << "\t# vertices = " << cmask->numVtxs() << std::endl;
        return nullptr;
    }   // end if

    cmask->setFaces( flt.topology);
    return cmask;
}   // end registerMask


r3d::Mat4f MaskRegistration::calcMaskAlignment( const r3d::Mesh &mask)
{
    const MaskPtr mdata = maskData();
    const IntSet &vidxs = mdata->mask->mesh().vtxIds();
    r3d::MatX3f tgtVtxRows( vidxs.size(), 3);
    r3d::MatX3f mvtxs( vidxs.size(), 3);
    int i = 0;
    for ( int vidx : vidxs)
    {
        tgtVtxRows.row(i) = mdata->mask->mesh().vtx(vidx);
        mvtxs.row(i) = mask.vtx(vidx);
        i++;
    }   // end for
    return r3d::ProcrustesSuperimposition( tgtVtxRows, false)( mvtxs);
}   // end calcMaskAlignment
