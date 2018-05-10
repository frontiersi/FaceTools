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
#include <ObjModelMover.h>  // RFeatures
#include <VtkTools.h>       // RVTK
#include <algorithm>

using FaceTools::FaceModel;
using RFeatures::ObjModelMover;
using RFeatures::ObjModelKDTree;
using RFeatures::ObjModel;


// public
void FaceModel::setModel( ObjModel::Ptr m)
{
    updateData(m);
    // Update all attached views
    std::for_each( std::begin(_fcs), std::end(_fcs), [&](auto fc){ fc->view()->rebuild();});
}   // end setModel


// public
void FaceModel::transform( const cv::Matx44d& m)
{
    _orientation.rotate(m); // Just use the rotation sub-matrix
    const ObjModelMover mover(m);
    mover( _model); // Adjust vertices of the model in-place
    _landmarks.transform(m);    // Transform the landmarks
    updateData(_model);
    // Update all attached views
    vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK(m);
    std::for_each( std::begin(_fcs), std::end(_fcs), [&](auto fc){ fc->transformView(vm);});
}   // end transform


// private
void FaceModel::updateData( ObjModel::Ptr m)
{
    _model = m;
    _kdtree = ObjModelKDTree::create(m);
    FaceTools::translateLandmarksToSurface( _kdtree, _landmarks);   // Ensure landmarks remapped to surface
}   // end updateData


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

void FaceModel::updateMesh( const ObjModel::Ptr model)
{
    _facalc.reset();
    _udist = NULL;
    _cdist = NULL;
    _omd->setObject(model); // Rebuilds internal kd-tree and resets curvature map to NULL (landmark info not changed!)
    _omd->shiftLandmarksToSurface();    // Ensures landmarks stay at surface
    _cmetrics = NULL;
    buildCurvature();
    buildDistanceMaps();
    emit meshUpdated();
}   // end updateMesh


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
