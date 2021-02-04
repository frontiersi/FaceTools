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
#include <boost/filesystem/path.hpp>
#include <boost/functional/hash.hpp>
//#include <thread>
using FaceTools::MaskRegistration;
using FaceTools::Vis::FV;
using FMM = FaceTools::FileIO::FaceModelManager;


MaskRegistration::MaskData MaskRegistration::s_mask;
QReadWriteLock MaskRegistration::s_lock;
MaskRegistration::Params MaskRegistration::s_params;


MaskRegistration::MaskData::MaskData() : mask(nullptr) {}


MaskRegistration::Params::Params()
    : k(3), flagThresh(0.9f), eqPushPull(false),
      kappa(4.0f), useOrient(true), numInlierIts(10),
      smoothK(80), sigmaSmooth(3.0f),
      numViscousStart(100), numViscousEnd(1),
      numElasticStart(100), numElasticEnd(1),
      numUpdateIts(20)
{
}   // end ctor


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


std::shared_ptr<const MaskRegistration::MaskData> MaskRegistration::maskData()
{
    s_lock.lockForRead();
    return std::shared_ptr<const MaskData>( &s_mask, []( const MaskData*){ s_lock.unlock();});
}   // end maskData


void MaskRegistration::setParams( const MaskRegistration::Params &prms) { s_params = prms;}


r3d::Mesh::Ptr MaskRegistration::registerMask( const r3d::Mesh &targetMesh)
{
    assert( maskLoaded());
    if ( !maskLoaded())
    {
        std::cerr << "[ERROR] FaceTools::Action::MaskRegistration::registerMask: Mask not loaded!" << std::endl;
        return nullptr;
    }   // end if

    // Clone the loaded mask
    s_lock.lockForRead();
    r3d::Mesh::Ptr mask = s_mask.mask->mesh().deepCopy();
    s_lock.unlock();

    //std::cout << "Calculating affine alignment (rigid + scaling) of mask to target face..." << std::endl;

    rNonRigid::Mesh floating;
    floating.features = mask->toFeatures( floating.topology, true/*use transformed*/);
    floating.flags = rNonRigid::FlagVec::Ones( floating.features.rows());   // Use all vertices on the mask

    rNonRigid::Mesh target;
    target.features = targetMesh.toFeatures( target.topology/*unused*/, true/*use transformed*/);
    target.flags = rNonRigid::FlagVec::Ones( target.features.rows());   // Use all vertices on the target

    const Mat4f T1 = rNonRigid::RigidRegistration()( floating, target);

    //std::cout << "Adding rigid transform to mask and reobtaining features..." << std::endl;
    mask->addTransformMatrix( T1);
    floating.features = mask->toFeatures( floating.topology, true/*use transformed*/);

    //std::cout << "Calculating non-rigid registration of mask to target face..." << std::endl;
    rNonRigid::NonRigidRegistration( s_params.k, s_params.flagThresh, s_params.eqPushPull,
                                     s_params.kappa, s_params.useOrient, s_params.numInlierIts,
                                     s_params.smoothK, s_params.sigmaSmooth,
                                     s_params.numViscousStart, s_params.numViscousEnd,
                                     s_params.numElasticStart, s_params.numElasticEnd,
                                     s_params.numUpdateIts)( floating, target);
    //rNonRigid::FastDeformRegistration( s_params.numUpdateIts)( floating, target);

    //std::cout << "Creating point cloud from registered vertices..." << std::endl;
    // On return, the vertices of feats are in correspondence with the surface of the model.
    // Make a new r3d::Mesh object from the surface registered points.
    r3d::Mesh::Ptr cmask = r3d::Mesh::fromVertices( floating.features.leftCols(3));
    //std::cout << "Setting mesh topology..." << std::endl;
    cmask->setFaces( floating.topology);
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
