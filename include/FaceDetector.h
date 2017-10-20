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

#ifndef FACE_TOOLS_FACE_DETECTOR_H
#define FACE_TOOLS_FACE_DETECTOR_H

#include <string>
#include <vector>
#include <Viewer.h> // RVTK
#include "Landmarks.h"
#include "ObjMetaData.h"
#include "FeaturesDetector.h"


namespace FaceTools
{

class FaceTools_EXPORT FaceDetector
{
public:
    typedef boost::shared_ptr<FaceDetector> Ptr;
    static Ptr create( const std::string& haarCascadesModelDir,
                       const std::string& faceShapeLandmarksModel,
                       float orientationRange=700.0f,
                       float detectionRange=288.0f,
                       bool showDebug=false
                       );

    // Find orientation and landmarks. Builds kd-tree as part of the process.
    // Returns false if any part of the process fails. Use err() to get the
    // error message corresponding to the failure.
    bool detect( ObjMetaData::Ptr);

    // Return error message relating to last detect() fail.
    const std::string& err() const { return _err;}

private:
    FeaturesDetector::Ptr _featuresDetector;
    class Impl;
    Impl* _impl;
    const float _orientationRange;
    const float _detRng;
    const bool _showDebug;
    std::string _err;

    bool findOrientation( RVTK::Viewer::Ptr, ObjMetaData::Ptr);
    bool findLandmarks( RVTK::Viewer::Ptr, ObjMetaData::Ptr);

    FaceDetector( FeaturesDetector::Ptr, Impl*, float, float, bool);
    ~FaceDetector();
    FaceDetector( const FaceDetector&);     // NO COPY
    void operator=( const FaceDetector&);   // NO COPY
    class Deleter;
};  // end class


}   // end namespace

#endif
