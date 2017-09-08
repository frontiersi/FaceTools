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
    : _objmeta(md)
{
    updateMesh( md->getObject());
}   // end ctor


// public
FaceModel::~FaceModel()
{
    updateMesh(ObjModel::Ptr());
}   // end dtor


// public
void FaceModel::setSaveFilepath( const std::string& filepath)
{
    _lastsave = filepath;
    emit onChangedSaveFilepath( this);
}   // end setSaveFilepath


// public
bool FaceModel::hasBoundary() const
{
    return _objmeta->getBoundary() != NULL;
}   // end hasBoundary


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
        _objmeta->setLandmark(lm, *pos);    // Set landmark with new position or add if not present
    else if ( _objmeta->hasLandmark(lm))
        _objmeta->deleteLandmark(lm);
    else
        noerr = false;

    if ( noerr)
        emit onLandmarkUpdated( lm, pos);

    return noerr;
}   // end updateLandmark


// public slot
bool FaceModel::updateLandmarkMeta( const std::string& lm, bool visible, bool movable, bool deletable)
{
    if ( !_objmeta->hasLandmark(lm))
        return false;
    FaceTools::Landmarks::Landmark* landmark = _objmeta->getLandmarkMeta(lm);
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
    if ( !_objmeta->hasLandmark( FaceTools::Landmarks::NASAL_TIP))
        return false;

    const ObjModel::Ptr model = _objmeta->getObject();
    if ( _facalc.getFaceAngles().empty()) // Obtain face angles
    {
        RFeatures::ObjModelTriangleMeshParser tparser( model);
        tparser.addTriangleParser( &_facalc);

        const cv::Vec3f& nt = _objmeta->getLandmark( FaceTools::Landmarks::NASAL_TIP);
        const int vidx = _objmeta->getKDTree()->find( nt);
        const int fidx = *model->getFaceIds(vidx).begin();  // Get connected polygon to nose tip
        tparser.parse( fidx, cv::Vec3f(0,0,1));
    }   // end if

    // Propagate from nearest point on the face.
    const int vidx = _objmeta->getKDTree()->find(v);

    const RFeatures::ObjModelFastMarcher::SpeedFunctor uniformSpeedFunctor;
    _udist = RFeatures::ObjModelFastMarcher::create( model, &uniformSpeedFunctor, &_facalc.getFaceAngles());
    _udist->propagateFront( vidx);

    const FaceTools::CurvatureSpeedFunctor curvSpeedFunctor( _objmeta->getCurvatureMap());
    _cdist = RFeatures::ObjModelFastMarcher::create( model, &curvSpeedFunctor, &_facalc.getFaceAngles());
    _cdist->propagateFront( vidx);
    return true;
}   // end updateDistanceMaps


// public slot
bool FaceModel::detectFace( FaceDetector::Ptr faceDetector)
{
    _err = "";
    if ( !faceDetector->findOrientation( _objmeta))
    {
        _err = "Unable to determine facial orientation!";
        return false;
    }   // end if

    if ( !faceDetector->findLandmarks( _objmeta))
    {
        _err = "Complete set of landmarks not found!";
        return false;
    }   // end if

    _isDetected = true;
    emit onFaceDetected();
    return true;
}   // end detectFace


// public slot
void FaceModel::updateBoundary( const std::vector<cv::Vec3f>& loop)
{
    _objmeta->setBoundary(loop);
    emit onBoundaryUpdated();
}   // end updateBoundary


// public slot
bool FaceModel::cropToBoundary( const cv::Vec3f& sv)
{
    _err = "";
    ObjModel::Ptr cmodel = _objmeta->cropToBoundary( sv);
    if ( !cmodel)
    {
        _err = "Failed to crop model on set boundary!";
        return false;
    }   // end if

    updateMesh( cmodel);
    emit onCropped();
    return true;
}   // end cropToBoundary


// public slot
void FaceModel::transformToStandardPosition()
{
    cv::Vec3f nvec, uvec;
    _objmeta->getOrientation( nvec, uvec);

    // Make complimentary on correcting axes
    nvec *= -1.0f;
    uvec *= -1.0f;
    nvec[2] *= -1.0f;
    uvec[1] *= -1.0f;

    const cv::Vec3d fc = FaceTools::calcFaceCentre(_objmeta);
    RFeatures::ObjModelMover rmat( nvec, uvec, -fc);
    _objmeta->transform( rmat.getTransformMatrix());
    // Need to reset because the face/vertex normals have changed
    // orientation even though their relative positions haven't.
    // Distance map data and face angles don't need recalculating however.
    reset( _objmeta->getObject());
    _isAligned = true;
    emit onTransformed();
}   // end transformToStandardPosition


// private
void FaceModel::reset( const ObjModel::Ptr model)
{
    _objmeta->setObject(model); // Resets internal kd-tree and curvature map if present
    _objmeta->rebuildKDTree();

    _cmetrics.reset();
    // If nosetip landmark available, update the curvature metrics
    if ( _objmeta->hasLandmark( FaceTools::Landmarks::NASAL_TIP))
    {
        const cv::Vec3f& v = _objmeta->getLandmark( FaceTools::Landmarks::NASAL_TIP);
        const int vidx = _objmeta->getKDTree()->find( v);
        _objmeta->rebuildCurvatureMap( vidx);
        _cmetrics = RFeatures::ObjModelCurvatureMetrics::create( _objmeta->getCurvatureMap());
    }   // end if
    _isAligned = false;
    _isDetected = false;
}   // end reset
