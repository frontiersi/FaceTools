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
#include <CurvatureSpeedFunctor.h>
#include <FaceTools.h>
using FaceTools::FaceModel;
using FaceTools::ObjMetaData;
using RFeatures::ObjModel;


// public
FaceModel::FaceModel( ObjMetaData::Ptr md) : QObject(), _omd(md)
{}   // end ctor


// public
void FaceModel::setFilePath( const std::string& fpath)
{
    std::string oldpath = getFilePath();
    _omd->setObjectFile( fpath);
    if ( oldpath != fpath)
        emit metaUpdated();
}   // end setFilePath


// public
const std::string& FaceModel::getFilePath() const
{
    return _omd->getObjectFile();
}   // end getFilePath


// public
const RFeatures::ObjModelCurvatureMetrics::Ptr FaceModel::getCurvatureMetrics() const { return _cmetrics;}


// public
const boost::unordered_map<int,double>* FaceModel::getUniformDistanceMap() const
{
    if ( _udist == NULL)
        return NULL;
    return &_udist->getCrossings();
}   // end getUniformDistanceMap


// public
const boost::unordered_map<int,double>* FaceModel::getCurvDistanceMap() const
{
    if ( _cdist == NULL)
        return NULL;
    return &_cdist->getCrossings();
}   // end getCurvDistanceMap


// public
bool FaceModel::updateLandmark( const std::string& lm, const cv::Vec3f* pos)
{
    bool noerr = true;
    if ( pos != NULL)
        _omd->setLandmark(lm, *pos);    // Set landmark with new position or add if not present
    else if ( _omd->hasLandmark(lm))
        _omd->deleteLandmark(lm);
    else
        noerr = false;

    if ( noerr)
        emit metaUpdated();
    return noerr;
}   // end updateLandmark


// public
void FaceModel::updateLandmark( const FaceTools::Landmarks::Landmark& lmk)
{
    _omd->setLandmark(lmk);
    emit metaUpdated();
}   // end updateLandmark


// public
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
    emit metaUpdated();
}   // end updateMesh


// private
void FaceModel::buildCurvature()
{
    if ( _omd->getCurvatureMap() != NULL)
        return;

    // If nosetip landmark available, update the curvature metrics
    if ( _omd->getObject() != NULL && _omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP))
    {
        const cv::Vec3f& v = _omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);
        const int vidx = _omd->getKDTree()->find( v);
        _omd->rebuildCurvatureMap( vidx);
        const RFeatures::ObjModelCurvatureMap::Ptr cmap = _omd->getCurvatureMap();
        assert( cmap != NULL);
        _cmetrics = RFeatures::ObjModelCurvatureMetrics::create( cmap);
    }   // end if
}   // end buildCurvature


// private
void FaceModel::buildDistanceMaps()
{
    if ( _omd->getObject() == NULL || !_omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP))
        return;

    const cv::Vec3f& nt = _omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);
    const int vidx = _omd->getKDTree()->find( nt);
    const ObjModel::Ptr model = _omd->getObject();

    if ( _facalc.getFaceAngles().empty()) // Obtain face angles
    {
        RFeatures::ObjModelTriangleMeshParser tparser( model);
        tparser.addTriangleParser( &_facalc);
        const int fidx = *model->getFaceIds(vidx).begin();  // Get connected polygon to nose tip
        tparser.parse( fidx, cv::Vec3f(0,0,1));
    }   // end if

    /*
    // Check the angles
    const RFeatures::FaceAngles& fangles = _facalc.getFaceAngles();
    const IntSet& fids = model->getFaceIds();
    foreach ( int fid, fids)
    {
        const RFeatures::VertexAngles& va = fangles.at(fid);
        const int* vidxs = model->getFaceVertices(fid);
        const double v0 = va.at(vidxs[0]);
        const double v1 = va.at(vidxs[1]);
        const double v2 = va.at(vidxs[2]);
        if ( fabs(v0 + v1 + v2 - CV_PI) > 0.0000001)
            std::cerr << "F_" << fid << ") " << v0 << ", " << v1 << ", " << v2 << std::endl;
    }   // end foreach
    */

    // Propagate from nose tip
    const RFeatures::ObjModelFastMarcher::SpeedFunctor uniformSpeedFunctor;
    _udist = RFeatures::ObjModelFastMarcher::create( model, &uniformSpeedFunctor, &_facalc.getFaceAngles());
    _udist->propagateFront( vidx);

    const FaceTools::CurvatureSpeedFunctor curvSpeedFunctor( _omd->getCurvatureMap());
    _cdist = RFeatures::ObjModelFastMarcher::create( model, &curvSpeedFunctor, &_facalc.getFaceAngles());
    _cdist->propagateFront( vidx);
}   // end buildDistanceMaps
