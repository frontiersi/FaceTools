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
using FaceTools::FaceModel;
using RFeatures::Transformer;
using RFeatures::ObjModelKDTree;
using RFeatures::ObjModelInfo;
using RFeatures::ObjModel;


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
    if ( _minfo)
    {
        _kdtree = ObjModelKDTree::create( _minfo->model().get());
        FaceTools::translateLandmarksToSurface( kdtree(), _landmarks);   // Ensure landmarks remapped to surface
    }   // end if

    _flagViewUpdate = true; // Will cause FaceActions to propagate data changes to all associated FaceControls
    return _minfo != NULL;
}   // end updateData


// public
void FaceModel::transform( const cv::Matx44d& m)
{
    _orientation.rotate( m);     // Just use the rotation sub-matrix
    Transformer(m).transform( _minfo->model()); // Adjust vertices of the model in-place
    _landmarks.transform(m);     // Transform the landmarks
    updateData();
}   // end transform


// public
const ObjModelKDTree& FaceModel::kdtree() const { return *_kdtree.get();}
const ObjModelInfo& FaceModel::info() const { return *_minfo.get();}


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
