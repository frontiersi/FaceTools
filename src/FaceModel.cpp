/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceControl.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <FaceView.h>
#include <VtkTools.h>       // RVTK
#include <algorithm>
#include <cassert>
using FaceTools::PathSet;
using FaceTools::FaceModel;
using FaceTools::LandmarkSet;
using FaceTools::FaceModelViewer;
using RFeatures::ObjModelKDTree;
using RFeatures::ObjModelInfo;
using RFeatures::ObjModel;

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

}   // end namespace


FaceModel::FaceModel( RFeatures::ObjModelInfo::Ptr minfo)
    : _saved(false), _description(""), _source("")
{
    assert(minfo);
    _landmarks = LandmarkSet::create();
    _paths = PathSet::create();
    update(minfo);
}   // end ctor


FaceModel::FaceModel()
    : _saved(false), _description(""), _source("")
{
    _landmarks = LandmarkSet::create();
    _paths = PathSet::create();
}   // end ctor


// public
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

    _kdtree = ObjModelKDTree::create( _minfo->cmodel());
    FaceTools::translateLandmarksToSurface( _kdtree, _landmarks);   // Ensure landmarks remapped to surface
    _paths->recalculate( _kdtree);                             // Ensure stored paths remap to the new surface.
    calculateBounds();

    _saved = false;
    return true;
}   // end update


// public
void FaceModel::transform( const cv::Matx44d& m)
{
    assert(_minfo);
    _orientation.rotate( m);    // Just use the rotation sub-matrix
    _landmarks->transform(m);   // Transform the landmarks
    _paths->transform(m);       // Transform the paths
    RFeatures::Transformer transformer(m);
    transformer.transform( _minfo->model()); // Adjust vertices of the model in-place
    _minfo->rebuildInfo();
    _kdtree = ObjModelKDTree::create( _minfo->cmodel());
    calculateBounds();

    vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK(m);
    for ( FaceControl* fc : _fcs)
    {
        fc->view()->pokeTransform( vm);
        fc->view()->fixTransform(); // "Hard" transform of VTK data.
    }   // end for

    _saved = false;
}   // end transform


// private
void FaceModel::calculateBounds()
{
    assert(_minfo);
    // Get the bounds for each of the model's components
    const RFeatures::ObjModelComponentFinder& components = _minfo->components();
    const int nc = (int)components.size();
    _cbounds.resize(nc);
    for ( int c = 0; c < nc; ++c)
        _cbounds[c] = getComponentBounds( _minfo->cmodel(), *components.componentBounds(c));
}   // end calculateBounds


// public
void FaceModel::lockForWrite() { _mutex.lockForWrite();}
void FaceModel::lockForRead() const { _mutex.lockForRead();}
void FaceModel::unlock() const { _mutex.unlock();}


// public
bool FaceModel::hasMetaData() const
{
    bool hmd = !landmarks()->empty();
    return hmd;
}   // end hasMetaData


// public
void FaceModel::updateRenderers() const
{
    FaceViewerSet fvs = _fcs.viewers();
    std::for_each( std::begin(fvs), std::end(fvs), [](auto v){ v->updateRender();});
}   // end updateRenderers


/*
const boost::unordered_map<int,double>* FaceModel::getUniformDistanceMap() const
{
    if ( _udist == NULL)
        return NULL;
    return &_udist->getCrossings();
}   // end getUniformDistanceMap


const boost::unordered_map<int,double>* FaceModel::getCurvDistanceMap() const
{
    if ( _cdist == NULL)
        return NULL;
    return &_cdist->getCrossings();
}   // end getCurvDistanceMap


void FaceModel::buildDistanceMaps()
{
    if ( _omd->getObject() == NULL || !_omd->landmarks()->hasLandmark( FaceTools::Landmarks::NASAL_TIP))
        return;

    const cv::Vec3f& nt = _omd->landmarks()->getLandmark( FaceTools::Landmarks::NASAL_TIP);
    const int vidx = _omd->getKDTree()->find( nt);
    const ObjModel::Ptr model = _omd->getObject();

    if ( _facalc.getFaceAngles().empty()) // Obtain face angles
    {
        RFeatures::ObjModelTriangleMeshParser tparser( model);
        tparser.addTriangleParser( &_facalc);
        const int fidx = *model->getFaceIds(vidx).begin();  // Get connected polygon to nose tip
        tparser.parse( fidx, cv::Vec3f(0,0,1));
    }   // end if

    // Propagate from nose tip
    const RFeatures::ObjModelFastMarcher::SpeedFunctor uniformSpeedFunctor;
    _udist = RFeatures::ObjModelFastMarcher::create( model, &uniformSpeedFunctor, &_facalc.getFaceAngles());
    _udist->propagateFront( vidx);

    const FaceTools::CurvatureSpeedFunctor curvSpeedFunctor( _omd->getCurvatureMap());
    _cdist = RFeatures::ObjModelFastMarcher::create( model, &curvSpeedFunctor, &_facalc.getFaceAngles());
    _cdist->propagateFront( vidx);
}   // end buildDistanceMaps
*/
