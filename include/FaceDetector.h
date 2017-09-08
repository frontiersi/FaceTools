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
#include "Landmarks.h"
#include "ObjMetaData.h"
#include "ModelViewer.h"
#include "FeaturesDetector.h"


namespace FaceTools
{

class FaceTools_EXPORT FaceDetector
{
public:
    typedef boost::shared_ptr<FaceDetector> Ptr;
    static Ptr create( const std::string& haarCascadesModelDir,
                       const std::string& faceShapeLandmarksModel);

    // Detect and set the NASAL_TIP, L_EYE_CENTRE, and R_EYE_CENTRE landmarks,
    // and set the orientation vectors (normal and up vector) given these points.
    // Return false iff the orientation points can't be found.
    bool findOrientation( ObjMetaData::Ptr);

    // Find remaining landmarks. Only call AFTER orientation discovered.
    bool findLandmarks( ObjMetaData::Ptr);

private:
    FeaturesDetector::Ptr _featuresDetector;
    class Impl;
    Impl* _impl;
    ModelViewer *_viewer;   // Offscreen scene rendering

    FaceDetector( FeaturesDetector::Ptr, Impl*);
    ~FaceDetector();
    FaceDetector( const FaceDetector&);     // NO COPY
    void operator=( const FaceDetector&);   // NO COPY
    class Deleter;
};  // end class


}   // end namespace

#endif
