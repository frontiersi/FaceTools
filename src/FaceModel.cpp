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
using RFeatures::ObjModelKDTree;
using RFeatures::ObjModelPolyUnfolder;
using RFeatures::ObjModelTriangleMeshParser;
using RFeatures::ObjModelInfo;
using RFeatures::ObjModel;


// public static
QString FaceModel::LENGTH_UNITS("mm");


namespace {

cv::Vec6d getComponentBounds( const ObjModel* model, const cv::Vec6i& bounds)
{
    const cv::Vec3f& xmin = model->vtx(bounds[0]);
    const cv::Vec3f& xmax = model->vtx(bounds[1]);
    const cv::Vec3f& ymin = model->vtx(bounds[2]);
    const cv::Vec3f& ymax = model->vtx(bounds[3]);
    const cv::Vec3f& zmin = model->vtx(bounds[4]);
    const cv::Vec3f& zmax = model->vtx(bounds[5]);
    return cv::Vec6d( xmin[0], xmax[0], ymin[1], ymax[1], zmin[2], zmax[2]);
}   // end getComponentBounds


/*
// Create and return a bounds vector from a set of points in the order set by toPoints.
cv::Vec6d toBounds( const std::vector<cv::Vec3d>& pts)
{
    assert(pts.size() == 8);
    cv::Vec6d bounds;
    bounds[0] = pts[0][0];  // xmin (0,3,4,7 suitable)
    bounds[1] = pts[1][0];  // xmax (1,2,5,6 suitable)
    bounds[2] = pts[2][1];  // ymin (2,3,6,7 suitable)
    bounds[3] = pts[0][1];  // ymax (0,1,4,5 suitable)
    bounds[4] = pts[0][2];  // zmin (0,1,2,3 suitable)
    bounds[5] = pts[4][2];  // zmax (4,5,6,7 suitable)
    return bounds;
}   // end toBounds


// Return the 8 points of a cuboid from the given bounds.
std::vector<cv::Vec3d> toPoints( const cv::Vec6d& bds)
{
    std::vector<cv::Vec3d> pts(8);
    pts[0] = cv::Vec3d( bds[0], bds[3], bds[4]); // Left, top, back
    pts[1] = cv::Vec3d( bds[1], bds[3], bds[4]); // Right, top, back
    pts[2] = cv::Vec3d( bds[1], bds[2], bds[4]); // Right, bottom, back
    pts[3] = cv::Vec3d( bds[0], bds[2], bds[4]); // Left, bottom, back
    pts[4] = cv::Vec3d( bds[0], bds[3], bds[5]); // Left, top, front
    pts[5] = cv::Vec3d( bds[1], bds[3], bds[5]); // Right, top, front
    pts[6] = cv::Vec3d( bds[1], bds[2], bds[5]); // Right, bottom, front
    pts[7] = cv::Vec3d( bds[0], bds[2], bds[5]); // Left, bottom, front
    return pts;
}   // end toPoints
*/


// Find and return index to largest rectangular volume from the given vector of bounds.
int findLargest( const std::vector<cv::Vec6d>& bounds)
{
    size_t j = 0;
    double maxA = 0;    // Max area
    size_t n = bounds.size();
    for ( size_t i = 0; i < n; ++i)
    {
        const cv::Vec6d& b = bounds[i];
        double a = (b[1] - b[0]) * (b[3] - b[2]) * (b[5] - b[4]);
        if ( a > maxA)
        {
            maxA = a;
            j = i;
        }   // end if
    }   // end for
    return int(j);
}   // end findLargest



// Return true if the cuboids specified with given edge extents intersect.
bool intersect( const cv::Vec6d& a, const cv::Vec6d& b)
{
    bool xAinB = ((a[0] >= b[0] && a[0] <= b[1]) || (a[1] >= b[0] && a[1] <= b[1]));    // x edges of A in B
    bool xBinA = ((b[0] >= a[0] && b[0] <= a[1]) || (b[1] >= a[0] && b[1] <= a[1]));    // x edges of B in A
    bool yAinB = ((a[2] >= b[2] && a[2] <= b[3]) || (a[3] >= b[2] && a[3] <= b[3]));    // y edges of A in B
    bool yBinA = ((b[2] >= a[2] && b[2] <= a[3]) || (b[3] >= a[2] && b[3] <= a[3]));    // y edges of B in A
    bool zAinB = ((a[4] >= b[4] && a[4] <= b[5]) || (a[5] >= b[4] && a[5] <= b[5]));    // y edges of A in B
    bool zBinA = ((b[4] >= a[4] && b[4] <= a[5]) || (b[5] >= a[4] && b[5] <= a[5]));    // y edges of B in A

    bool xint = xAinB || xBinA;
    bool yint = yAinB || yBinA;
    bool zint = zAinB || zBinA;
    return xint && yint && zint;
}   // end intersect

}   // end namespace


FaceModel::FaceModel( RFeatures::ObjModelInfo::Ptr minfo)
    : _saved(false), _description(""), _source(""),
      _age(0), _sex(FaceTools::UNKNOWN_SEX), _ethnicity(""), _cdate( QDate::currentDate()),
      _centreSet(false), _centre(0,0,0)
{
    assert(minfo);
    _landmarks = LandmarkSet::create();
    _paths = PathSet::create();
    update(minfo);
}   // end ctor


FaceModel::FaceModel()
    : _saved(false), _description(""), _source(""),
      _age(0), _sex(FaceTools::UNKNOWN_SEX), _ethnicity(""), _cdate( QDate::currentDate()),
      _centreSet(false), _centre(0,0,0)
{
    _landmarks = LandmarkSet::create();
    _paths = PathSet::create();
}   // end ctor


bool FaceModel::update( ObjModelInfo::Ptr nfo)
{
    if (!nfo)
        nfo = _minfo;

    _minfo = nfo;
    assert(_minfo);
    if ( !_minfo || !_minfo->is2DManifold())
    {
        std::cerr << "[ERROR] FaceTools::FaceModel::update: null or non-manifold ObjModelInfo passed in!" << std::endl;
        return false;
    }   // end if

    updateMeta();

    _saved = false;
    return true;
}   // end update


void FaceModel::transform( const cv::Matx44d& m)
{
    assert(_minfo);
    _orientation.rotate( m);    // Just use the rotation sub-matrix
    _landmarks->transform(m);   // Transform the landmarks
    _paths->transform(m);       // Transform the paths
    RFeatures::Transformer transformer(m);
    transformer.transform( _minfo->model()); // Adjust vertices of the model in-place
    transformer.transform( _centre);    // Transform the centre point in-place

    _minfo->rebuildInfo();

    updateMeta();

    vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK(m);
    pokeTransform( vm);
    fixTransform( vm);  // Fix the transform across all actors associated with this model.
    _saved = false;
}   // end transform


void FaceModel::pokeTransform( vtkMatrix4x4* m)
{
    // Get all of the layers. We do this instead of allowing each FaceView to poke its own
    // layers because some layers are currently unapplied for some FaceViews. However, we
    // still need to poke the main actor in the FaceView.
    VisualisationLayers vlayers;
    for ( Vis::FV* fv : _fvs)
    {
        fv->actor()->PokeMatrix(m);
        const VisualisationLayers& vl = fv->visualisations();
        vlayers.insert( std::begin(vl), std::end(vl));
    }   // end for
    for ( BaseVisualisation* vis : vlayers)
        std::for_each( std::begin(_fvs), std::end(_fvs), [=]( Vis::FV* fv){ vis->pokeTransform( fv, m);});
}   // end pokeTransform


// private
void FaceModel::fixTransform( vtkMatrix4x4* m)
{
    VisualisationLayers vlayers;
    for ( Vis::FV* fv : _fvs)
    {
        RVTK::transform( fv->actor(), m);
        const VisualisationLayers& vl = fv->visualisations();
        vlayers.insert( std::begin(vl), std::end(vl));
    }   // end for
    for ( BaseVisualisation* vis : vlayers)
        std::for_each( std::begin(_fvs), std::end(_fvs), [=]( Vis::FV* fv){ vis->fixTransform( fv);});

    // Reset transforms back to the identity matrix.
    vtkNew<vtkMatrix4x4> I;
    I->Identity();
    pokeTransform( I);
}   // end fixTransform


cv::Vec3f FaceModel::findClosestSurfacePoint( const cv::Vec3f& v) const
{
    lockForRead();
    const cv::Vec3f cv = FaceTools::toSurface(&*_kdtree, v);
    unlock();
    return cv;
}   // end findClosestSurfacePoint


namespace {

cv::Vec6d calcSuperBounds( const std::vector<cv::Vec6d>& bset)
{
    cv::Vec6d bounds(DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX);
    for ( const cv::Vec6d& b : bset)
    {
        bounds[0] = std::min(bounds[0], b[0]);  // xmin
        bounds[1] = std::max(bounds[1], b[1]);  // xmax
        bounds[2] = std::min(bounds[2], b[2]);  // ymin
        bounds[3] = std::max(bounds[3], b[3]);  // ymax
        bounds[4] = std::min(bounds[4], b[4]);  // zmin
        bounds[5] = std::max(bounds[5], b[5]);  // zmax
    }   // end for
    return bounds;
}   // end calcSuperBounds

}   // end namespace


// private
void FaceModel::updateMeta()
{
    calculateBounds();
    const ObjModel* model = _minfo->cmodel();
    _kdtree = ObjModelKDTree::create( model);
    _landmarks->moveToSurface( &*_kdtree);   // Remap landmarks to surface
    _paths->recalculate( &*_kdtree);   // Ensure stored paths remap to the new surface.
}   // end updateMeta


// private
void FaceModel::calculateBounds()
{
    assert(_minfo);
    // Get the bounds for each of the model's components
    const RFeatures::ObjModelComponentFinder& components = _minfo->components();
    const size_t nc = components.size();
    _cbounds.resize(nc);
    for ( size_t c = 0; c < nc; ++c)
        _cbounds[c] = getComponentBounds( _minfo->cmodel(), *components.componentBounds(int(c)));

    if ( !centreSet())
    {   // calculate centre of largest component.
        const cv::Vec6d& bd = _cbounds[ size_t(findLargest( _cbounds))];
        _centre = cv::Vec3f( float(bd[0] + bd[1])/2, float(bd[2] + bd[3])/2, float(bd[4] + bd[5])/2);
    }   // end if

    _sbounds = calcSuperBounds( _cbounds);
}   // end calculateBounds


void FaceModel::lockForWrite() { _mutex.lockForWrite();}
void FaceModel::lockForRead() const { _mutex.lockForRead();}
void FaceModel::unlock() const { _mutex.unlock();}


bool FaceModel::hasMetaData() const
{
    return !_description.isEmpty()
        || !_source.isEmpty()
        || _age != 0.0
        || _sex != FaceTools::UNKNOWN_SEX
        || !_ethnicity.isEmpty()
        || _cdate != QDate::currentDate()
        || _centreSet
        || _orientation != RFeatures::Orientation()
        || !_landmarks->empty()
        || !_paths->empty()
        || !_metrics.empty()
        || !_metricsL.empty()
        || !_metricsR.empty();
}   // end hasMetaData


void FaceModel::clearLandmarks()
{
    _orientation = RFeatures::Orientation();
    _centreSet = false;
    _centre = cv::Vec3f(0,0,0);
    _landmarks->clear();
    _metrics.reset();
    _metricsL.reset();
    _metricsR.reset();
    setSaved(false);
}   // end clearLandmarks


void FaceModel::updateRenderers() const
{
    FMVS fvs = _fvs.dviewers();
    std::for_each( std::begin(fvs), std::end(fvs), [](FMV* v){ v->updateRender();});
}   // end updateRenderers


double FaceModel::translateToSurface( cv::Vec3f& pos) const
{
    int notused;
    cv::Vec3f fv;
    int vidx = _kdtree->find(pos);
    const RFeatures::ObjModelSurfacePointFinder spfinder( _kdtree->model());
    double sdiff = spfinder.find( pos, vidx, notused, fv);
    pos = fv;
    return sdiff;
}   // end translateToSurface


bool FaceModel::supersIntersect( const FaceModel& fm) const
{
    return intersect( superBounds(), fm.superBounds());
}   // end supersIntersect


void FaceModel::clearPhenotypes()
{
    if ( !_phenotypes.empty())
        _phenotypes.clear();
}   // end clearPhenotypes


void FaceModel::addPhenotype( int hid)
{
    if ( _phenotypes.count(hid) == 0)
        _phenotypes.insert(hid);
}   // end addPhenotype
