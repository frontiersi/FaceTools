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

#ifndef FACE_TOOLS_FACE_MODEL_H
#define FACE_TOOLS_FACE_MODEL_H

#include "FaceDetector.h"
#include <QObject>
#include <ObjModelCurvatureMetrics.h>   // RFeatures
#include <ObjModelFastMarcher.h>        // RFeatures
#include "ObjMetaData.h"

namespace FaceTools
{

class FaceTools_EXPORT FaceModel : public QObject
{ Q_OBJECT
public:
    explicit FaceModel( ObjMetaData::Ptr);
    virtual ~FaceModel();

    ObjMetaData::Ptr getObjectMeta() const { return _objmeta;}
    const RFeatures::ObjModelCurvatureMetrics::Ptr getCurvatureMetrics() const; // May be NULL
    const boost::unordered_map<int,double>* getUniformDistanceMap() const;  // May be NULL
    const boost::unordered_map<int,double>* getCurvDistanceMap() const;  // May be NULL

    // Sets the last saved filename and causes onChangedSaveFilename to fire.
    void setSaveFilepath( const std::string& filepath);
    const std::string& getSaveFilepath() const { return _lastsave;}

    // If detectFace returns false, get the nature of the error here.
    const std::string& err() const { return _err;}
    bool hasUndos() const { return false;}  // TODO add undo/redo functionality
    bool isAligned() const { return _isAligned;}
    bool isDetected() const { return _isDetected;}
    bool hasBoundary() const;

signals:
    // Notify of landmark addition, repositioning, or deletion, or just
    // changes to its metadata. If position is NULL, landmark was removed.
    void onLandmarkUpdated( const std::string&, const cv::Vec3f*);
    void onLandmarkSelected( const std::string&, bool);     // Fires when the given landmark is selected.
    void onLandmarkHighlighted( const std::string&, bool);  // Fires when the given landmark is highlighted.
    void onChangedSaveFilepath( FaceModel*);          // Fires after changing save filepath
    void onMeshUpdated();       // Fires whenever the underlying model morphology has been modified.
    void onFaceDetected();      // Fires upon successful detection of the face.
    void onBoundaryUpdated();   // Fires upon user updating of the facial boundary.
    void onCropped();           // Fires after cropping and *after* onMeshUpdated emitted.
    void onTransformed();       // Fires after face transformed in space.
    void onClearedUndos( FaceModel*);  // Fires when undos reset (TODO)

public slots:
    // Update the model mesh. Resets mesh data. If the nasal tip is defined,
    // rebuilds all curvature data (the distance maps are generated on demand).
    // Causes onMeshUpdated to be emitted.
    void updateMesh( const RFeatures::ObjModel::Ptr);

    // Update the distance maps. Returns true on success. If false returned,
    // the NASAL_TIP landmark first needs to be provided so that a call to
    // updateMesh causes the face angles to be calculated.
    bool updateDistanceMaps( const cv::Vec3f&);

    // Select/deselect a landmark. Causes onSelectedLandmark to fire.
    void selectLandmark( const std::string&, bool);

    // Highlight/de-highlight a landmark. Causes onHighlightLandmark to fire.
    void highlightLandmark( const std::string&, bool);

    // Add, remove, or reposition a landmark.
    // Landmark is added if not present and pos != NULL.
    // Landmark is repositioned if present and pos != NULL.
    // Landmark is removed if present and pos == NULL.
    // Returns true in above three cases and signals onLandmarkUpdated.
    // Returns false if landmark not present and pos == NULL.
    bool updateLandmark( const std::string&, const cv::Vec3f*);

    // Update the landmark's meta data, returning true iff landmark exists.
    // Parameters in order are landmark name, visible, movable, deletable.
    // Fires onLandmarkMetaUpdated on success.
    bool updateLandmarkMeta( const std::string&, bool, bool, bool);

    // Set the orientation of the face (its norm and up vector) and detect landmarks.
    // Re-detects if landmarks already present. Returns true on successful
    // detection of landmarks and causes onFaceDetected to be emitted.
    // If not successful, user may need to adjust camera viewpoint.
    bool detectFace( FaceDetector::Ptr);

    // Update the list of boundary vertices. Fires onBoundaryUpdated.
    void updateBoundary( const std::vector<cv::Vec3f>&);

    // Update the mesh to exclude everything outside the currently defined boundary.
    // Causes both onMeshUpdated and onCropped to be emitted in that order.
    bool cropToBoundary( const cv::Vec3f&);

    // Translate the face so that it's "centre" is at the world origin and
    // orient the face so that it's normal is incident with the world's +Z
    // vector and it's up vector incident with the world's +Y vector.
    // Fires onTransformed afterwards.
    void transformToStandardPosition();

private:
    ObjMetaData::Ptr _objmeta;
    RFeatures::ObjModelFaceAngleCalculator _facalc;
    RFeatures::ObjModelCurvatureMetrics::Ptr _cmetrics;
    RFeatures::ObjModelFastMarcher::Ptr _udist;
    RFeatures::ObjModelFastMarcher::Ptr _cdist;
    std::string _err;
    std::string _lastsave;
    bool _isAligned;
    bool _isDetected;
    void reset( RFeatures::ObjModel::Ptr);
    FaceModel( const FaceModel&);               // No copy
    FaceModel& operator=( const FaceModel&);    // No copy
};  // end class

}   // end namespace

#endif
