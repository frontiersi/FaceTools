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
using FaceTools::FMVS;
using FaceTools::Vis::VisualisationLayers;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using namespace RFeatures;


// public static
QString FaceModel::LENGTH_UNITS("mm");
int FaceModel::MAX_MANIFOLDS(1);


FaceModel::FaceModel( ObjModel::Ptr model)
    : _savedMeta(false), _savedModel(false), _notes(""), _source(""), _studyId(""),
      _dob( QDate::currentDate()), _sex(FaceTools::MALE_SEX | FaceTools::FEMALE_SEX),
      _methnicity(0), _pethnicity(0), _cdate( QDate::currentDate())
{
    assert(model);
    _landmarks = LandmarkSet::create();
    _paths = PathSet::create();
    update(model);
}   // end ctor


FaceModel::FaceModel()
    : _savedMeta(false), _savedModel(false), _notes(""), _source(""), _studyId(""),
      _dob( QDate::currentDate()), _sex(FaceTools::MALE_SEX | FaceTools::FEMALE_SEX),
      _methnicity(0), _pethnicity(0), _cdate( QDate::currentDate())
{
    _landmarks = LandmarkSet::create();
    _paths = PathSet::create();
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
    _paths->recalculate( this);   // Ensure stored paths remap to the new surface.
    //moveLandmarksToSurface();   // Actions must now do this explicitly
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
    if ( !_landmarks->empty())
        T = _landmarks->orientation().asMatrix( _landmarks->fullMean());
    const size_t nm = _manifolds->count();
    _bnds.resize(nm+1);
    for ( size_t i = 0; i < nm; ++i)
    {
        const IntSet& mvidxs = _manifolds->manifold(int(i))->vertices(*_model);
        _bnds[i+1] = ObjModelBounds::create( *_model, T, &mvidxs);
    }   // end for

    // If no landmarks, make the bounds entry at zero the union of the manifold bounding cuboids
    // otherwise the bounds at zero represents a box around the head of the model.
    if ( _landmarks->empty())
    {
        _bnds[0] = _bnds[1]->deepCopy();
        for ( size_t i = 2; i < nm+1; ++i)
            _bnds[0]->encompass(*_bnds[i]);
    }   // end if
    else
    {
        // Get the landmark mean, superior and inferior means in standard position
        cv::Vec3f lm = _landmarks->fullMean();
        cv::Vec3f sm = _landmarks->superiorMean();
        cv::Vec3f im = _landmarks->inferiorMean();
        const cv::Matx44d iT = T.inv();
        RFeatures::transform( iT, lm);
        RFeatures::transform( iT, sm);
        RFeatures::transform( iT, im);
        const double xlen = 2.2*cv::norm(_landmarks->eyeVec());
        const double ylen = 2.2*cv::norm(sm-im);
        const double zlen = ylen;
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
    const cv::Matx44d T = _landmarks->orientation().asMatrix( _landmarks->fullMean());
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

    if ( !_landmarks->empty())
    {
        _landmarks->addTransformMatrix(T);
        setMetaSaved( false);
    }   // end if

    if ( !_paths->empty())
    {
        _paths->transform(T);
        setMetaSaved( false);
    }   // end if

    setModelSaved( false);
}   // end addTransformMatrix


void FaceModel::fixOrientation()
{
    _model->fixTransformMatrix();
    _kdtree = ObjModelKDTree::create( *_model);
    _makeOrientationBounds();
    setModelSaved( false);
}   // end fixOrientation


void FaceModel::setMetaSaved( bool s) { _savedMeta = s;}
void FaceModel::setModelSaved( bool s) { _savedModel = s;}


cv::Vec3f FaceModel::icentre() const
{
    cv::Vec3f c;
    if (!_landmarks->empty())
        c = _landmarks->fullMean();
    else
    {
        const cv::Matx44d& T = _bnds[0]->transformMatrix();
        c = cv::Vec3f( float(T(0,3)), float(T(1,3)), float(T(2,3)));
    }   // end else
    return c;
}   // end icentre


Orientation FaceModel::orientation() const
{
    Orientation on;
    if ( !_bnds.empty())
        on = Orientation( _bnds[0]->transformMatrix());
    return on;
}   // end orientation


cv::Vec3f FaceModel::findClosestSurfacePoint( const cv::Vec3f& v) const
{
    return FaceTools::toSurface( this, v);
}   // end findClosestSurfacePoint


int FaceModel::findVertex( const cv::Vec3f& v) const { return _kdtree->find(v);}


void FaceModel::lockForWrite() { _mutex.lockForWrite();}
void FaceModel::lockForRead() const { _mutex.lockForRead();}
void FaceModel::unlock() const { _mutex.unlock();}


void FaceModel::setNotes( const QString& n)
{
    if ( n != _notes)
    {
        _notes = n;
        setMetaSaved(false);
    }   // end if
}   // end setNotes


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
    return !_notes.isEmpty()
        || !_source.isEmpty()
        || !_studyId.isEmpty()
        || _dob != QDate::currentDate()
        || _sex == FaceTools::MALE_SEX
        || _sex == FaceTools::FEMALE_SEX
        || _methnicity > 0
        || _pethnicity > 0
        || _cdate != QDate::currentDate()
        || !_landmarks->empty()
        || !_paths->empty()
        || !_metrics.empty()
        || !_metricsL.empty()
        || !_metricsR.empty();
}   // end hasMetaData


int FaceModel::faceManifoldId() const
{
    const LandmarkSet& lmks = landmarks();
    int mid = -1;
    if ( !lmks.empty())
    {
        const cv::Vec3f v = lmks.posSomeMedial();
        const int svidx = findVertex( v);
        const int fx = *model().faces(svidx).begin(); // Some attached polygon
        mid = manifolds().manifoldId(fx); // Manifold ID
    }   // end if
    return mid;
}   // end faceManifoldId


bool FaceModel::hasMetric( int mid) const
{
    return _metrics.ids().count(mid) > 0 || _metricsL.ids().count(mid) > 0 || _metricsR.ids().count(mid) > 0;
}   // end hasMetric


void FaceModel::clearMetrics()
{
    _metrics.reset();
    _metricsL.reset();
    _metricsR.reset();
}   // end clearMetrics


void FaceModel::setLandmarks( LandmarkSet::Ptr lmks)
{
    if ( !_landmarks->empty() || !lmks->empty())
    {
        setMetaSaved(false);
        _landmarks = lmks;
        _syncOrientationBounds();
    }   // end if
}   // end setLandmarks


bool FaceModel::setLandmarkPosition(int lid, const cv::Vec3f &pos, FaceTools::FaceLateral flat)
{
    setMetaSaved(false);
    const bool okay = _landmarks->set( lid, pos, flat);    // Adds if not already present
    assert(okay);
    _syncOrientationBounds();
    return okay;
}   // end setLandmarkPosition


void FaceModel::moveLandmarksToSurface()
{
    if ( !_landmarks->empty())
    {
        setMetaSaved(false);
        _landmarks->moveToSurface( this);
        _syncOrientationBounds();
    }   // end if
}   // end moveLandmarksToSurface


void FaceModel::swapLandmarkLaterals()
{
    if ( !_landmarks->empty())
    {
        setMetaSaved(false);
        _landmarks->swapLaterals();
        _syncOrientationBounds();
    }   // end if
}   // end swapLandmarkLaterals


void FaceModel::setPaths(PathSet::Ptr pths)
{
    if ( !_paths->empty() || !pths->empty())
    {
        setMetaSaved(false);
        _paths = pths;
    }   // end if
}   // end setPaths


int FaceModel::addPath( const cv::Vec3f& pos)
{
    setMetaSaved(false);
    const int aid = _paths->addPath(pos);
    _paths->setActivePath(aid);
    return aid;
}   // end addPath


bool FaceModel::removePath( int pid)
{
    bool rval = false;
    if ( _paths->has(pid))
    {
        setMetaSaved(false);
        rval = _paths->removePath(pid);
    }   // end if
    return rval;
}   // end removePath


bool FaceModel::renamePath( int pid, const QString &nm)
{
    bool rval = false;
    if ( _paths->has(pid))
    {
        setMetaSaved(false);
        rval = _paths->renamePath(pid, nm.toStdString());
    }   // end if
    return rval;
}   // end renamePath


bool FaceModel::setPathPosition( int pid, const cv::Vec3f& pos, int h)
{
    Path* path = _paths->path(pid);
    if ( path)
    {
        if ( h == 0)
            *path->vtxs.begin() = pos;
        else
            *path->vtxs.rbegin() = pos;
        path->recalculate( this);
        setMetaSaved(false);
    }   // end if
    return path != nullptr;
}   // end setPathPosition


void FaceModel::updateRenderers() const
{
    FMVS fvs = _fvs.viewers();
    std::for_each( std::begin(fvs), std::end(fvs), [](FMV* v){ v->updateRender();});
}   // end updateRenderers


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
