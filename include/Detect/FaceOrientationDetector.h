/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_DETECT_FACE_ORIENTATION_DETECTOR_H
#define FACE_TOOLS_DETECT_FACE_ORIENTATION_DETECTOR_H

#include "FeaturesDetector.h"
#include "FaceShapeLandmarks2DDetector.h"
#include <LandmarkSet.h>
#include <OffscreenModelViewer.h>   // RVTK
#include <ObjModelKDTree.h>         // RFeatures
#include <Orientation.h>            // RFeatures

namespace FaceTools { namespace Detect {

class FaceTools_EXPORT FaceOrientationDetector
{
public:
    // Make an offscreen viewer for detecting the orientation of a face.
    // After orientation has been detected, the viewer can be returned and used
    // for follow-up detection work. Detection uses the Viola and Jones HaarCascades
    // detector (see FaceTools::FeaturesDetector and RFeatures::HaarCascadeDetector).
    // Provide the initial detection range with d used to scale how closely the camera
    // is positioned to the face based on the detected distance between the eyes e
    // with the distance formula rng * e/d.
    FaceOrientationDetector( const RFeatures::ObjModelKDTree* kdt, float rng=700.0f, float d=0.30f);

    // Update only the landmarks with the given IDs.
    // If left as default, allows all landmarks to be updated.
    void setLandmarksToUpdate( const IntSet& ul=COMPLETE_INT_SET);

    // Detect and place into the given landmarks set the detected features of the face and set orientation.
    bool detect( Landmark::LandmarkSet&);

    // Returns the orientation of the face after return from detect.
    RFeatures::Orientation orientation() const;

    // If detect returns false, return the error string.
    const std::string& error() const { return _err;}

private:
    void setCameraToFace(float);
    // Orient the camera in the offscreen viewer to position the face in an
    // upright pose at a standardised distance to cause the face to fill the
    // viewer in preparation for landmark detection. Returns detection range.
    float orient();

    RVTK::OffscreenModelViewer _vwr;
    const RFeatures::ObjModelKDTree* _kdt;
    float _orng, _dfact;
    cv::Vec3f _nvec, _v0, _v1;
    std::string _err;
    IntSet _ulmks;
};  // end class

}}   // end namespace

#endif
