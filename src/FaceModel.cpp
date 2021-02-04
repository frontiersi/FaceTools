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

#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::PathSet;
using FaceTools::FaceModel;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FaceAssessment;
using FaceTools::Vis::FV;
using FaceTools::Vec3f;
using FaceTools::Mat4f;


// public static
QString FaceModel::LENGTH_UNITS("mm");
int FaceModel::MAX_MANIFOLDS(1);

namespace {
static const float MATRIX_PRECISION = 1e-4f;
}   // end namespace


FaceModel::FaceModel( r3d::Mesh::Ptr mesh)
    : _savedMeta(false), _savedModel(false), _source(""), _studyId(""), _subjectId(""), _imageId(""),
      _dob( QDate::currentDate()), _sex(FaceTools::UNKNOWN_SEX),
      _methnicity(0), _pethnicity(0), _cdate( QDate::currentDate())
{
    assert(mesh);
    setAssessment( FaceAssessment::create( 0));
    update( mesh, true, false);
}   // end ctor


FaceModel::FaceModel()
    : _savedMeta(false), _savedModel(false), _source(""), _studyId(""), _subjectId(""), _imageId(""),
      _dob( QDate::currentDate()), _sex(FaceTools::UNKNOWN_SEX),
      _methnicity(0), _pethnicity(0), _cdate( QDate::currentDate())
{
    setAssessment( FaceAssessment::create(0));
}   // end ctor


void FaceModel::update( r3d::Mesh::Ptr mesh, bool updateConnectivity, bool settleLandmarks, int maxManifolds)
{
    assert( mesh);
    assert( mesh != _mesh);

    if ( updateConnectivity)
    {
        /*
        static const std::string imsg = "[INFO] FaceTools::FaceModel::update: ";
        const size_t rverts = mesh->removeDisconnectedVertices();
        if ( rverts > 0)
            std::cerr << imsg << "Removed " << rverts << " disconnected vertices" << std::endl;
        */

        // Ensure that vertices are in sequential order.
        if ( !mesh->hasSequentialIds())
            mesh = mesh->repackedCopy();

        if ( mesh->numMats() > 1)  // Merge materials?
            mesh->mergeMaterials();

        if ( maxManifolds <= 0)
            maxManifolds = MAX_MANIFOLDS;

        r3d::Manifolds::Ptr manf = r3d::Manifolds::create( *mesh);
        if ( int(manf->count()) > maxManifolds)
        {
            //std::cerr << imsg << "Reducing from " << manf->count() << " to " << maxManifolds << " manifolds..." << std::endl;
            mesh = manf->reduceManifolds( maxManifolds);
            manf = r3d::Manifolds::create( *mesh);
        }   // end if

        const int nm = static_cast<int>( manf->count());
        for ( int i = 0; i < nm; ++i)
        {
            manf->at(i).boundaries();  // Causes boundary edges to be calculated
            //const auto& bnds = manf->at(i).boundaries();  // Causes boundary edges to be calculated
            //std::cerr << " - Manifold " << i << " has " << bnds.count() << " boundary edges" << std::endl;
        }   // end for
        _manifolds = manf;
    }   // end updateConnectivity

    _mesh = mesh;
    _kdtree = r3d::KDTree::create( *_mesh);
    if ( settleLandmarks)
        _moveToSurface();
    remakeBounds();
}   // end update


void FaceModel::fixTransformMatrix()
{
    r3d::Mesh::Ptr nmesh = _mesh->deepCopy();
    nmesh->fixTransformMatrix();
    update( nmesh, false, false);
    if ( _mask)
    {
        r3d::Mesh::Ptr nmask = _mask->deepCopy();
        nmask->fixTransformMatrix();
        setMask( nmask);
    }   // end if
}   // end fixTransformMatrix


void FaceModel::remakeBounds()
{
    assert(_manifolds);
    const Mat4f T = transformMatrix();
    const size_t nm = _manifolds->count();
    _bnds.resize(nm+1);
    for ( size_t i = 0; i < nm; ++i)
    {
        const IntSet& mvidxs = _manifolds->at(int(i)).vertices();
        // Providing current alignment in creation of bounds means they are
        // created with this transform in mind so when inverse of the transform
        // is applied, the bounds will be upright and in standard position.
        _bnds[i+1] = r3d::Bounds::create( *_mesh, T, &mvidxs);
    }   // end for

    _bnds[0] = _bnds[1]->deepCopy();
    for ( size_t i = 2; i < nm+1; ++i)
        _bnds[0]->encompass(*_bnds[i]);

    setMetaSaved( false);
    setModelSaved( false);
}   // end remakeBounds


Mat4f FaceModel::transformMatrix() const { return _mesh->transformMatrix();}
Mat4f FaceModel::inverseTransformMatrix() const { return _mesh->inverseTransformMatrix();}


bool FaceModel::isAligned() const { return transformMatrix().isIdentity( MATRIX_PRECISION);}


void FaceModel::_syncBoundsToAlignment()
{
    Mat4f T = transformMatrix();
    for ( auto& b : _bnds)
        b->setTransformMatrix(T);
}   // end _syncBoundsToAlignment


void FaceModel::addTransformMatrix( const Mat4f& T)
{
    assert( !T.isZero());
    _mesh->addTransformMatrix( T);
    if ( _mask)
        _mask->addTransformMatrix( T);
    for ( auto& ass : _ass)
        ass.get()->transform(T);
    // Have to do it this way because the model may not yet have landmarks defined.
    for ( auto& b : _bnds)
        b->addTransformMatrix(T);
    setMetaSaved( false);
    setModelSaved( false);
}   // end addTransformMatrix


void FaceModel::addView( FV *fv) { _fvs.insert(fv);}
void FaceModel::eraseView( FV* fv) { _fvs.erase(fv);}

void FaceModel::setMetaSaved( bool s) { _savedMeta = s;}
void FaceModel::setModelSaved( bool s) { _savedModel = s;}


Vec3f FaceModel::centreFront() const
{
    assert(!_bnds.empty());
    const r3d::Vec6f cns = _bnds[0]->cornersAs6f();  // Untransformed corners
    const Vec3f cfront( 0.5f * (cns[0] + cns[1]), 0.5f * (cns[2] + cns[3]), cns[5]);
    return r3d::transform( transformMatrix(), cfront);
}   // end centreFront


Vec3f FaceModel::findClosestSurfacePoint( const Vec3f& v) const { return FaceTools::toSurface( kdtree(), v);}
int FaceModel::findVertex( const Vec3f& v) const { return kdtree().find(v);}


void FaceModel::lockForWrite() { _mutex.lockForWrite();}
void FaceModel::lockForRead() const { _mutex.lockForRead();}
void FaceModel::unlock() const { _mutex.unlock();}


FaceModel::RPtr FaceModel::scopedReadLock() const
{
    _mutex.lockForRead();
    return RPtr( this, []( const FaceModel *fm){ fm->_mutex.unlock();});
}   // end scopedReadLock


FaceModel::WPtr FaceModel::scopedWriteLock()
{
    _mutex.lockForWrite();
    return WPtr( this, []( FaceModel *fm){ fm->_mutex.unlock();});
}   // end scopedWriteLock


void FaceModel::setSource( const QString& s)
{
    if ( _source != s)
    {
        _source = s;
        setMetaSaved(false);
    }   // end if
}   // end setSource


void FaceModel::setStudyId( const QString& s)
{
    if ( _studyId != s)
    {
        _studyId = s;
        setMetaSaved(false);
    }   // end if
}   // end setStudyId


void FaceModel::setSubjectId( const QString& s)
{
    if ( _subjectId != s)
    {
        _subjectId = s;
        setMetaSaved(false);
    }   // end if
}   // end setSubjectId


void FaceModel::setImageId( const QString& s)
{
    if ( _imageId != s)
    {
        _imageId = s;
        setMetaSaved(false);
    }   // end if
}   // end setImageId


void FaceModel::setDateOfBirth( const QDate& d)
{
    if ( _dob != d)
    {
        _dob = d;
        setMetaSaved(false);
    }   // end if
}   // end setDateOfBirth


bool FaceModel::isSubjectMatch( const FM *fm) const
{
    return subjectId() == fm->subjectId()
        && dateOfBirth() == fm->dateOfBirth()
        && maskHash() == fm->maskHash();
}   // end isSubjectMatch


void FaceModel::setMaternalEthnicity( int t)
{
    if ( _methnicity != t)
    {
        _methnicity = t;
        setMetaSaved(false);
    }   // end if
}   // end setMaternalEthnicity


void FaceModel::setPaternalEthnicity( int t)
{
    if ( _pethnicity != t)
    {
        _pethnicity = t;
        setMetaSaved(false);
    }   // end if
}   // end setPaternalEthnicity


void FaceModel::setCaptureDate( const QDate& d)
{
    if ( _cdate != d)
    {
        _cdate = d;
        setMetaSaved(false);
    }   // end if
}   // end setCaptureDate


void FaceModel::setSex( int8_t s)
{
    if ( _sex != s)
    {
        _sex = s;
        setMetaSaved(false);
    }   // end if
}   // end setSex


bool FaceModel::hasMetaData() const
{
    bool assContent = false;
    for ( const auto& a : _ass)
    {
        if (a->hasContent())
        {
            assContent = true;
            break;
        }   // end if
    }   // end for

    return assContent
        || !_source.isEmpty()
        || !_studyId.isEmpty()
        || !_subjectId.isEmpty()
        || !_imageId.isEmpty()
        || _dob != QDate::currentDate()
        || _sex != FaceTools::UNKNOWN_SEX
        || _methnicity != 0
        || _pethnicity != 0
        || _cdate != QDate::currentDate();
}   // end hasMetaData


int FaceModel::faceManifoldId() const
{
    for ( const auto& a : _ass)
    {
        const LandmarkSet& lmks = a.get()->landmarks();
        if ( !lmks.empty())
        {
            const int svidx = findVertex( lmks.pos( Landmark::PRN));
            const int fx = *_mesh->faces(svidx).begin(); // Some attached face 
            return manifolds().fromFaceId(fx); // Manifold ID
        }   // end if
    }   // end for
    return -1;
}   // end faceManifoldId


float FaceModel::toSurface( Vec3f& pos) const
{
    int notused;
    Vec3f fv;
    int vidx = _kdtree->find(pos);
    const r3d::SurfacePointFinder spfinder( *_mesh);
    float sdiff = spfinder.find( pos, vidx, notused, fv);
    pos = fv;
    return sdiff;
}   // end toSurface


void FaceModel::setMaskHash( size_t h)
{
    if ( _maskHash != h)
        setMetaSaved(false);
    _maskHash = h;
}   // end setMaskHash


void FaceModel::setMask( r3d::Mesh::Ptr mask, bool copyTexture)
{
    if ( mask != _mask)
        setMetaSaved(false);

    _mask = mask;
    if ( !_mask)
    {
        _mkdtree = nullptr;
        _maskHash = 0;
        return;
    }   // end if

    _mask->setTransformMatrix( _mesh->transformMatrix());
    _mkdtree = r3d::KDTree::create( *_mask);

    // Copy texture info from this model's mesh into the mask
    if ( copyTexture && _mesh->hasMaterials())
    {
        int sfid;
        Vec3f sfv;
        const r3d::SurfacePointFinder spfinder( *_mesh);
        const int N = int(mask->numVtxs());

        // Compute all the texture coordinates for each vertex in the mask
        std::vector<r3d::Vec2f> uvs(N);
        for ( int i = 0; i < N; ++i)
        {
            const Vec3f &v = mask->vtx(i);
            int svidx = findVertex(v);
            spfinder.find( v, svidx, sfid, sfv);
            if ( sfid >= 0)
                uvs[i] = _mesh->calcTextureCoords( sfid, sfv);
            else
            {
                sfid = *_mesh->faces(svidx).begin();
                uvs[i] = _mesh->faceUV( sfid, _mesh->face(sfid).index(svidx));
            }   // end else
        }   // end for

        const int mid = mask->addMaterial( _mesh->texture( *_mesh->materialIds().begin()));

        const int NF = int(mask->numFaces());
        for ( int i = 0; i < NF; ++i)
        {
            const int *fvids = mask->fvidxs(i);
            mask->setOrderedFaceUVs( mid, i, uvs[fvids[0]], uvs[fvids[1]], uvs[fvids[2]]);
        }   // end for
    }   // end if
}   // end setMask


void FaceModel::setCurrentAssessment( int id)
{
    const bool doRemakeBounds = _cass != nullptr && _manifolds != nullptr && _cass->id() != id;
    _cass = _ass.value(id);
    if ( doRemakeBounds)
        remakeBounds();
}   // end setCurrentAssessment


void FaceModel::setAssessment( FaceAssessment::Ptr ass)
{
    _ass[ass->id()] = ass;
    setCurrentAssessment( ass->id());
}   // end setAssessment


void FaceModel::eraseAssessment( int id)
{
    _ass.remove(id);
    if ( _cass && _cass->id() == id)
    {
        _cass = nullptr;
        if ( !_ass.empty())
        {
            _cass = _ass.first();
            remakeBounds();
        }   // end if
    }   // end if
}   // end eraseAssessment


FaceAssessment::CPtr FaceModel::assessment( int id) const { return id < 0 ? _cass : _ass[id];}
FaceAssessment::Ptr FaceModel::assessment( int id) { return id < 0 ? _cass : _ass[id];}


IntSet FaceModel::assessmentIds() const
{
    IntSet aids;
    for ( const auto& a : _ass)
        aids.insert(a.get()->id());
    return aids;
}   // end assessmentIds


const LandmarkSet& FaceModel::currentLandmarks() const { return _cass->landmarks();}

const PathSet& FaceModel::currentPaths() const { return _cass->paths();}
PathSet& FaceModel::currentPaths() { return _cass->paths();}

bool FaceModel::hasPaths() const { return !currentPaths().empty();}


void FaceModel::setLandmarks( const LandmarkSet &lmks)
{
    if ( _cass->setLandmarks( lmks))
        remakeBounds();
}   // end setLandmarks


bool FaceModel::hasLandmarks() const { return _cass->hasLandmarks();}


void FaceModel::setLandmarkPosition( int lid, FaceTools::FaceSide flat, const Vec3f &pos)
{
    _cass->landmarks().set( lid, pos, flat);
    remakeBounds();
}   // end setLandmarkPosition


void FaceModel::swapLandmarkLaterals()
{
    bool didswap = false;
    for ( auto& ass : _ass)
    {
        if ( ass.get()->hasLandmarks())
        {
            ass.get()->landmarks().swapLaterals();
            didswap = true;
        }   // end if
    }   // end for

    if ( didswap)
        remakeBounds();
}   // end swapLandmarkLaterals


bool FaceModel::_moveToSurface()
{
    bool setunsaved = false;
    for ( auto& ass : _ass)
    {
        if ( ass.get()->hasPaths() || ass.get()->hasLandmarks())
        {
            ass.get()->moveToSurface(this);
            setunsaved = true;
        }   // end if
    }   // end for
    return setunsaved;
}   // end _moveToSurface


void FaceModel::moveToSurface()
{
    if ( _moveToSurface())
        remakeBounds();
}   // end moveToSurface


LandmarkSet FaceModel::makeMeanLandmarksSet() const
{
    std::unordered_set<const LandmarkSet*> slmks;
    for ( const auto& a : _ass)
    {
        const LandmarkSet& lmks = a->landmarks();
        if ( !lmks.empty())
            slmks.insert( &lmks);
    }   // end for

    LandmarkSet mlmks( slmks);
    assert( _kdtree);
    mlmks.moveToSurface(this);
    return mlmks;
}   // end makeMeanLandmarksSet


int FaceModel::addPath( const Vec3f& pos)
{
    assert( _cass);
    setMetaSaved(false);
    return _cass->paths().addPath( pos);
}   // end addPath


void FaceModel::removePath( int pid)
{
    assert( _cass);
    if ( _cass->paths().removePath(pid))
        setMetaSaved(false);
}   // end removePath


void FaceModel::renamePath( int pid, const QString &nm)
{
    assert( _cass);
    if ( _cass->paths().renamePath( pid, nm.toStdString()))
        setMetaSaved(false);
}   // end renamePath

