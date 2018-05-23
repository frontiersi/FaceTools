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
#include <FaceTools.h>
#include <FaceView.h>
#include <VtkTools.h>       // RVTK
#include <algorithm>
#include <cassert>
using FaceTools::FaceModel;
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


// public
FaceModel::FaceModel() : _flagViewUpdate(false) {}


// public
bool FaceModel::updateData( ObjModel::Ptr m)
{
    if ( m)
    {
        _minfo = ObjModelInfo::create(m);
        if ( _minfo)
        {
            if ( m->getNumMaterials() > 1) // Merge materials if more than 1 texture map
            {
                std::cerr << "[INFO] FaceTools::FaceModel::updateData: Merged materials" << std::endl;
                m->mergeMaterials();
            }   // end if
        }   // end if
    }   // end if

    _kdtree = NULL;
    _cbounds.clear();
    if ( _minfo)
    {
        _kdtree = ObjModelKDTree::create( _minfo->cmodel());
        FaceTools::translateLandmarksToSurface( kdtree(), _landmarks);   // Ensure landmarks remapped to surface

        // Get the bounds for each of the model's components
        const RFeatures::ObjModelComponentFinder& components = _minfo->components();
        const int nc = (int)components.size();
        for ( int c = 0; c < nc; ++c)
        {
            const cv::Vec6i* bounds = components.componentBounds(c);
            assert(bounds);
            _cbounds.push_back( getComponentBounds( _minfo->cmodel(), *bounds));
        }   // end for
    }   // end if

    _flagViewUpdate = true; // Cause FaceActions to propagate data changes to all associated FaceControls
    return _minfo != NULL;
}   // end updateData


// public
void FaceModel::transform( const cv::Matx44d& m)
{
    _orientation.rotate( m);     // Just use the rotation sub-matrix
    _landmarks.transform(m);     // Transform the landmarks
    RFeatures::Transformer transformer(m);
    transformer.transform( _minfo->model()); // Adjust vertices of the model in-place
    updateData();
}   // end transform


// public
const ObjModelKDTree& FaceModel::kdtree() const { return *_kdtree.get();}
const ObjModelInfo* FaceModel::info() const { return _minfo.get();}


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


