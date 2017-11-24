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

#include <ObjModelCurvatureMetrics.h>   // RFeatures
#include <ObjModelFastMarcher.h>        // RFeatures
#include "ObjMetaData.h"                // FaceTools
#include <QObject>
#include <QMetaMethod>

namespace FaceTools
{

class FaceTools_EXPORT FaceModel : public QObject
{ Q_OBJECT
public:
    // Client MUST call updateMesh after instantiation to initialise curvature etc.
    explicit FaceModel( ObjMetaData::Ptr);
    virtual ~FaceModel(){}

    ObjMetaData::Ptr getObjectMeta() const { return _omd;}

    // Use these functions to set/get the filepath.
    void setFilePath( const std::string&);  // Will cause metaUpdated to be emitted.
    const std::string& getFilePath() const;

    const RFeatures::ObjModelCurvatureMetrics::Ptr getCurvatureMetrics() const; // May be NULL
    const boost::unordered_map<int,double>* getUniformDistanceMap() const;  // May be NULL
    const boost::unordered_map<int,double>* getCurvDistanceMap() const;  // May be NULL

    // Update the model mesh. Resets mesh data. If the nasal tip is defined,
    // rebuilds all curvature data (the distance maps are generated on demand).
    void updateMesh( const RFeatures::ObjModel::Ptr);

    // Add, remove, or reposition a landmark.
    // Landmark is added if not present and pos != NULL.
    // Landmark is repositioned if present and pos != NULL.
    // Landmark is removed if present and pos == NULL.
    // Returns false if landmark not present and pos == NULL.
    bool updateLandmark( const std::string&, const cv::Vec3f*);

    // Update the landmark data, overwritting the existing entry,
    // or adding a new one of it doesn't already exist. To delete
    // landmarks, use the above updateLandmark function.
    void updateLandmark( const Landmarks::Landmark&);

    // Update the distance maps. Returns true on success. If false returned,
    // the NASAL_TIP landmark first needs to be provided so that a call to
    // updateMesh causes the face angles to be calculated.
    //bool updateDistanceMaps( const cv::Vec3f&);

signals:
    void metaUpdated();     // Emitted when the model is changed.
    void meshUpdated();

private:
    ObjMetaData::Ptr _omd;
    RFeatures::ObjModelFaceAngleCalculator _facalc;
    RFeatures::ObjModelCurvatureMetrics::Ptr _cmetrics;
    RFeatures::ObjModelFastMarcher::Ptr _udist;
    RFeatures::ObjModelFastMarcher::Ptr _cdist;

    void buildCurvature();

    FaceModel( const FaceModel&);               // No copy
    FaceModel& operator=( const FaceModel&);    // No copy
};  // end class

}   // end namespace

#endif
