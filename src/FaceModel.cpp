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

#include <FaceModel.h>
#include <FaceModelViewer.h>
#include <BaseVisualisation.h>
#include <FaceTools.h>
#include <VtkTools.h>       // RVTK
#include <algorithm>
#include <cassert>
using FaceTools::PathSet;
using FaceTools::FaceModel;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FaceAssessment;
using FaceTools::FMVS;
using FaceTools::Vis::VisualisationLayers;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using namespace RFeatures;


// public static
QString FaceModel::LENGTH_UNITS("mm");
int FaceModel::MAX_MANIFOLDS(1);


FaceModel::FaceModel( ObjModel::Ptr model)
    : _savedMeta(false), _savedModel(false), _source(""), _studyId(""),
      _dob( QDate::currentDate()), _sex(FaceTools::MALE_SEX | FaceTools::FEMALE_SEX),
      _methnicity(0), _pethnicity(0), _cdate( QDate::currentDate())
{
    assert(model);
    setAssessment( FaceAssessment::create(0));
    update(model);
}   // end ctor


FaceModel::FaceModel()
    : _savedMeta(false), _savedModel(false), _source(""), _studyId(""),
      _dob( QDate::currentDate()), _sex(FaceTools::MALE_SEX | FaceTools::FEMALE_SEX),
      _methnicity(0), _pethnicity(0), _cdate( QDate::currentDate())
{
    setAssessment( FaceAssessment::create(0));
}   // end ctor


void FaceModel::update( ObjModel::Ptr model, bool updateConnectivity)
{
    if ( !model)
        model = _model;

    assert(model);

    if ( updateConnectivity)
    {
        static const std::string imsg = "[INFO] FaceTools::FaceModel::update: ";
        const size_t rverts = removeDisconnectedVertices( *model);
        if ( rverts > 0)
            std::cerr << imsg << "Removed " << rverts << " disconnected vertices" << std::endl;

        // Ensure that vertices are in sequential order.
        if ( !model->hasSequentialIds())
            model = model->repackedCopy(true);

        if ( model->numMats() > 1)  // Merge materials?
        {
            std::cerr << imsg << "Merging materials..." << std::endl;
            model->mergeMaterials();
        }   // end if

        ObjModelManifolds::Ptr manf = ObjModelManifolds::create( *model);
        if ( int(manf->count()) > MAX_MANIFOLDS)
        {
            std::cerr << imsg << "Reducing from " << manf->count() << " to " << MAX_MANIFOLDS << " manifolds..." << std::endl;
            model = manf->reduceManifolds( *model, MAX_MANIFOLDS);
            manf = ObjModelManifolds::create( *model);
        }   // end if

        const int nm = static_cast<int>( manf->count());

        std::cerr << imsg << "Finding boundaries..." << std::endl;
        for ( int i = 0; i < nm; ++i)
        {
            const auto& bnds = manf->manifold(i)->boundaries(*model);  // Causes boundary edges to be calculated
            std::cerr << " - Manifold " << i << " has " << bnds.count() << " boundary edges" << std::endl;
        }   // end for

        _manifolds = manf;
    }   // end updateConnectivity

    setModelSaved(false);
    _model = model;
    _kdtree = ObjModelKDTree::create( *_model);

    // Recalculate path positions on the model over all assessments
    for ( auto& ass : _ass)
        ass.get()->recalculatePaths(this);

    _makeOrientationBounds();
}   // end update


void FaceModel::_makeOrientationBounds()
{
    // Matrix transform to sync the bounds to is defined either by the model
    // without semantic orientation information (i.e. just assuming model bounds
    // along world axes), or by the landmarks if present.
    // Calculate bounds with respect to the orientation matrix (uses landmarks if set).
    cv::Matx44d T = _model->transformMatrix();  // Identity matrix normally
    //std::cerr << "_makeOrientationBounds: " << std::endl;
    //std::cerr << T << std::endl;
    const LandmarkSet& lmks = currentAssessment()->landmarks();
    if ( !lmks.empty())
        T = lmks.orientation().asMatrix( lmks.fullMean());
    const size_t nm = _manifolds->count();
    _bnds.resize(nm+1);
    for ( size_t i = 0; i < nm; ++i)
    {
        const IntSet& mvidxs = _manifolds->manifold(int(i))->vertices(*_model);
        _bnds[i+1] = ObjModelBounds::create( *_model, T, &mvidxs);
    }   // end for

    // If no landmarks, make the bounds entry at zero the union of the manifold bounding cuboids
    // otherwise the bounds at zero represents a box around the head of the model.
    if ( lmks.empty())
    {
        _bnds[0] = _bnds[1]->deepCopy();
        for ( size_t i = 2; i < nm+1; ++i)
            _bnds[0]->encompass(*_bnds[i]);
    }   // end if
    else
    {
        // Get the landmark mean, superior and inferior means in standard position
        cv::Vec3f lm = lmks.fullMean();
        cv::Vec3f sm = lmks.superiorMean();
        cv::Vec3f im = lmks.inferiorMean();
        const cv::Matx44d iT = T.inv();
        RFeatures::transform( iT, lm);
        RFeatures::transform( iT, sm);
        RFeatures::transform( iT, im);
        const double xlen = 2.5*cv::norm( lmks.eyeVec());
        const double ylen = 2.6*cv::norm(sm-im);
        const double zlen = 1.1 * ylen;
        cv::Vec3d minc(-xlen/2, -ylen/2, -0.8*zlen);
        cv::Vec3d maxc(xlen/2, ylen/2, 0.2*zlen);
        RFeatures::transform( T, minc);
        RFeatures::transform( T, maxc);
        _bnds[0] = ObjModelBounds::create( minc, maxc, T);
    }   // end else
}   // end _makeOrientationBounds


void FaceModel::addView( FV *fv)
{
    _fvs.insert(fv);
}   // end addView


void FaceModel::eraseView( FV* fv)
{
    _fvs.erase(fv);
}   // end eraseView


void FaceModel::_syncOrientationBounds()
{
    const LandmarkSet& lmks = currentAssessment()->landmarks();
    const cv::Matx44d T = lmks.orientation().asMatrix( lmks.fullMean());
    // Update the bounds
    for ( auto& b : _bnds)
        b->setTransformMatrix(T);
}   // end _syncOrientationBounds


void FaceModel::addTransformMatrix( const cv::Matx44d& T)
{
    _model->addTransformMatrix( T);
    _kdtree->addTransformMatrix( T);
    // Update the bounds
    for ( auto& b : _bnds)
        b->addTransformMatrix(T);

    bool addedTransform = true;
    for ( auto& a : _ass)
        addedTransform = a.get()->addTransform(T) && addedTransform;

    if ( addedTransform)
        setMetaSaved( false);
    setModelSaved( false);
}   // end addTransformMatrix


void FaceModel::fixOrientation()
{
    _model->fixTransformMatrix();
    for ( auto& a : _ass)
        a.get()->fixTransform();
    _kdtree = ObjModelKDTree::create( *_model);
    _makeOrientationBounds();
    setModelSaved( false);
}   // end fixOrientation


void FaceModel::setMetaSaved( bool s) { _savedMeta = s;}
void FaceModel::setModelSaved( bool s) { _savedModel = s;}


cv::Vec3f FaceModel::centre() const
{
    assert(!_bnds.empty());
    const cv::Matx44d& T = _bnds[0]->transformMatrix();
    return cv::Vec3f( float(T(0,3)), float(T(1,3)), float(T(2,3)));
}   // end centre


Orientation FaceModel::orientation() const
{
    assert(!_bnds.empty());
    return Orientation( _bnds[0]->transformMatrix());
}   // end orientation


cv::Vec3f FaceModel::findClosestSurfacePoint( const cv::Vec3f& v) const { return FaceTools::toSurface( this, v);}
int FaceModel::findVertex( const cv::Vec3f& v) const { return _kdtree->find(v);}

void FaceModel::lockForWrite() { _mutex.lockForWrite();}
void FaceModel::lockForRead() const { _mutex.lockForRead();}
void FaceModel::unlock() const { _mutex.unlock();}


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


void FaceModel::setDateOfBirth( const QDate& d)
{
    if ( _dob != d)
    {
        _dob = d;
        setMetaSaved(false);
    }   // end if
}   // end setDateOfBirth


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
    return _cass->hasContent()
        || !_source.isEmpty()
        || !_studyId.isEmpty()
        || _dob != QDate::currentDate()
        || _sex == FaceTools::MALE_SEX
        || _sex == FaceTools::FEMALE_SEX
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
            const cv::Vec3f v = lmks.posSomeMedial();
            const int svidx = findVertex( v);
            const int fx = *model().faces(svidx).begin(); // Some attached polygon
            return manifolds().manifoldId(fx); // Manifold ID
        }   // end if
    }   // end for
    return -1;
}   // end faceManifoldId


double FaceModel::translateToSurface( cv::Vec3f& pos) const
{
    int notused;
    cv::Vec3f fv;
    int vidx = _kdtree->find(pos);
    const ObjModelSurfacePointFinder spfinder( *_model);
    double sdiff = spfinder.find( pos, vidx, notused, fv);
    pos = fv;
    return sdiff;
}   // end translateToSurface


void FaceModel::setCurrentAssessment( int id)
{
    assert(_ass.count(id) > 0);
    _cass = _ass.value(id);
}   // end setCurrentAssessment


void FaceModel::setAssessment( FaceAssessment::Ptr ass)
{
    assert(ass);
    if ( !_cass || _cass->id() == ass->id())
        _cass = ass;
    _ass[ass->id()] = ass;
}   // end setAssessment


void FaceModel::eraseAssessment( int id)
{
    _ass.remove(id);
    if ( _cass && _cass->id() == id)
    {
        _cass = nullptr;
        if ( !_ass.empty())
            _cass = _ass.first();
    }   // end if
}   // end eraseAssessment


FaceAssessment::CPtr FaceModel::assessment( int id) const { return _ass[id];}
FaceAssessment::Ptr FaceModel::assessment( int id) { return _ass[id];}


IntSet FaceModel::assessmentIds() const
{
    IntSet aids;
    for ( const auto& a : _ass)
        aids.insert(a.get()->id());
    return aids;
}   // end assessmentIds


void FaceModel::setLandmarks( LandmarkSet::Ptr lmks)
{
    assert( _cass);
    if ( _cass->setLandmarks(lmks))
    {
        setMetaSaved(false);
        _syncOrientationBounds();
    }   // end if
}   // end setLandmarks


bool FaceModel::hasLandmarks() const
{
    for ( const auto& a : _ass)
        if ( a->hasLandmarks())
            return true;
    return false;
}   // end hasLandmarks


void FaceModel::setLandmarkPosition( int lid, FaceTools::FaceLateral flat, const cv::Vec3f &pos)
{
    assert( _cass);
    _cass->setLandmarkPosition( lid, flat, pos);
    setMetaSaved(false);
    _syncOrientationBounds();
}   // end setLandmarkPosition


void FaceModel::swapLandmarkLaterals()
{
    bool didswap = false;
    for ( auto& a : _ass)
        if ( a.get()->swapLandmarkLaterals())
            didswap = true;
    if ( didswap)
    {
        setMetaSaved(false);
        _syncOrientationBounds();
    }   // end if
}   // end swapLandmarkLaterals


void FaceModel::moveLandmarksToSurface()
{
    bool moved = false;
    for ( auto& a : _ass)
        if ( a.get()->moveLandmarksToSurface(this))
            moved = true;
    if ( moved)
    {
        setMetaSaved(false);
        _syncOrientationBounds();
    }   // end if
}   // end moveLandmarksToSurface


LandmarkSet::Ptr FaceModel::makeMeanLandmarksSet() const
{
    std::unordered_set<const LandmarkSet*> slmks;
    for ( const auto& a : _ass)
    {
        const LandmarkSet& lmks = a->landmarks();
        if ( !lmks.empty())
            slmks.insert( &lmks);
    }   // end for

    LandmarkSet::Ptr mlmks = LandmarkSet::createMean( slmks);
    if ( _kdtree)   // Possible if this function is called before a model is set.
        mlmks->moveToSurface(this);
    return mlmks;
}   // end makeMeanLandmarksSet


void FaceModel::setPaths( PathSet::Ptr pths)
{
    assert( _cass);
    if ( _cass->setPaths(pths))
        setMetaSaved(false);
}   // end setPaths


int FaceModel::addPath( const cv::Vec3f& pos)
{
    assert( _cass);
    setMetaSaved(false);
    return _cass->addPath( pos);
}   // end addPath


void FaceModel::removePath( int pid)
{
    assert( _cass);
    if ( _cass->removePath(pid))
        setMetaSaved(false);
}   // end removePath


void FaceModel::renamePath( int pid, const QString &nm)
{
    assert( _cass);
    if ( _cass->renamePath( pid, nm))
        setMetaSaved(false);
}   // end renamePath


void FaceModel::setPathPosition( int pid, int h, const cv::Vec3f& pos)
{
    assert( _cass);
    if ( _cass->setPathPosition( this, pid, h, pos))
        setMetaSaved(false);
}   // end setPathPosition
