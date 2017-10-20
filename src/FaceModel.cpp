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
using FaceTools::FaceDetector;
using FaceTools::ObjMetaData;
using RFeatures::ObjModel;
#include <QFile>


// public
FaceModel::FaceModel( ObjMetaData::Ptr md)
    : _omd(md)
{
    updateMesh( md->getObject());
    _isAligned = false;
    _isDetected = false;
    _faceCropRadiusFactor = 2.1;
}   // end ctor


// public
FaceModel::~FaceModel()
{
    updateMesh(ObjModel::Ptr());
}   // end dtor


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
void FaceModel::setSaveFilepath( const std::string& filepath)
{
    _lastsave = filepath;
    emit onChangedSaveFilepath( this);
}   // end setSaveFilepath


// public
void FaceModel::setFaceCropFactor( double G)
{
    _faceCropRadiusFactor = G;
    emit onSetFaceCropFactor( G);
}   // end setFaceCropFactor


// public slot
void FaceModel::selectLandmark( const std::string& lm, bool enable)
{
    emit onLandmarkSelected(lm, enable);
}   // end selectLandmark


// public slot
void FaceModel::highlightLandmark( const std::string& lm, bool enable)
{
    emit onLandmarkHighlighted(lm, enable);
}   // end highlightLandmark


// public slot
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
        emit onLandmarkUpdated( lm, pos);

    return noerr;
}   // end updateLandmark


// public slot
bool FaceModel::updateLandmarkMeta( const std::string& lm, bool visible, bool movable, bool deletable)
{
    if ( !_omd->hasLandmark(lm))
        return false;
    FaceTools::Landmarks::Landmark* landmark = _omd->getLandmarkMeta(lm);
    landmark->visible = visible;
    landmark->movable = movable;
    landmark->deletable = deletable;
    emit onLandmarkUpdated( lm, &landmark->pos);
    return true;
}   // end updateLandmarkMeta


// public slot
void FaceModel::updateMesh( const ObjModel::Ptr model)
{
    _facalc.reset();
    _udist.reset();
    _cdist.reset();
    reset( model);
    if ( model != NULL)
        emit onMeshUpdated();
}   // end updateMesh


// public slot
bool FaceModel::updateDistanceMaps( const cv::Vec3f& v)
{
    if ( !_omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP))
        return false;

    const ObjModel::Ptr model = _omd->getObject();
    if ( _facalc.getFaceAngles().empty()) // Obtain face angles
    {
        RFeatures::ObjModelTriangleMeshParser tparser( model);
        tparser.addTriangleParser( &_facalc);

        const cv::Vec3f& nt = _omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);
        const int vidx = _omd->getKDTree()->find( nt);
        const int fidx = *model->getFaceIds(vidx).begin();  // Get connected polygon to nose tip
        tparser.parse( fidx, cv::Vec3f(0,0,1));
    }   // end if

    // Propagate from nearest point on the face.
    const int vidx = _omd->getKDTree()->find(v);

    const RFeatures::ObjModelFastMarcher::SpeedFunctor uniformSpeedFunctor;
    _udist = RFeatures::ObjModelFastMarcher::create( model, &uniformSpeedFunctor, &_facalc.getFaceAngles());
    _udist->propagateFront( vidx);

    const FaceTools::CurvatureSpeedFunctor curvSpeedFunctor( _omd->getCurvatureMap());
    _cdist = RFeatures::ObjModelFastMarcher::create( model, &curvSpeedFunctor, &_facalc.getFaceAngles());
    _cdist->propagateFront( vidx);
    return true;
}   // end updateDistanceMaps


// public slot
bool FaceModel::detectFace( FaceDetector::Ptr faceDetector)
{
    _err = "";
    if ( !faceDetector->detect( _omd))
    {
        _err = faceDetector->err();
        return false;
    }   // end if

    _isDetected = true;

    assert( _omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP));
    const cv::Vec3f& v = _omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);
    const int vidx = _omd->getKDTree()->find( v);
    ObjModel::Ptr comp = FaceTools::getComponent( _omd->getObject(), vidx);
    updateMesh(comp);
    emit onFaceDetected();
    return true;
}   // end detectFace


// public slot
bool FaceModel::cropFace()
{
    _err = "";
    assert( _omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP));
    const double cropRadius = FaceTools::calcFaceCropRadius( _omd, _faceCropRadiusFactor);
    if ( cropRadius <= 0)
    {
        _err = "Unable to calculate face crop radius!";
        return false;
    }   // end if

    const int svid = _omd->getKDTree()->find( _omd->getLandmark( FaceTools::Landmarks::NASAL_TIP));
    const cv::Vec3f fc = FaceTools::calcFaceCentre(_omd);
    ObjModel::Ptr cmodel = FaceTools::crop( _omd->getObject(), fc, cropRadius, svid);
    if ( !cmodel)
    {
        _err = "Unable to crop face!";
        return false;
    }   // end if

    updateMesh( cmodel);
    emit onCropped();
    return true;
}   // end cropFace


// public slot
void FaceModel::transformToStandardPosition()
{
    cv::Vec3f nvec, uvec;
    _omd->getOrientation( nvec, uvec);

    // Make complimentary on correcting axes
    nvec *= -1.0f;
    uvec *= -1.0f;
    nvec[2] *= -1.0f;
    uvec[1] *= -1.0f;

    const cv::Vec3d fc = FaceTools::calcFaceCentre(_omd);
    RFeatures::ObjModelMover rmat( nvec, uvec, -fc);
    _omd->transform( rmat.getTransformMatrix());
    // Need to reset because the face/vertex normals have changed
    // orientation even though their relative positions haven't.
    // Distance map data and face angles don't need recalculating however.
    reset( _omd->getObject());
    _isAligned = true;
    emit onTransformed();
}   // end transformToStandardPosition


// private
void FaceModel::reset( const ObjModel::Ptr model)
{
    _omd->setObject(model); // Rebuilds internal kd-tree and resets curvature map to NULL
    _cmetrics.reset();
    buildCurvature();
}   // end reset


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
