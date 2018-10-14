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
#include <FaceTools.h>
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
    FaceOrientationDetector( const RFeatures::ObjModelKDTree::Ptr kdt, float rng=700.0f, float d=0.30f);

    RVTK::Viewer::Ptr offscreenViewer() const { return _vwr;}

    // Orient the camera in the offscreen viewer to position the face in an
    // upright pose at a standardised distance to cause the face to fill the
    // viewer in preparation for landmark detection.
    bool orient();

    // Detect and place into the given landmarks set the detected features of the
    // face. Call after orient(). This also provides a more accurate detection
    // of the face's orientation.
    bool detect( Landmark::LandmarkSet&);

    // Returns the orientation of the face after return from orient().
    RFeatures::Orientation orientation() const { return RFeatures::Orientation(_nvec, _uvec);}

    // Returns the detection range the camera is set to after return from orient().
    float detectRange() const { return _eprop * _dfact;}

    // If orient returns false, return the error string.
    const std::string& error() const { return _err;}

private:
    float detect3DEyePositionsFrom2D();
    void setCameraToFace();

    RVTK::Viewer::Ptr _vwr;
    const RFeatures::ObjModelKDTree::Ptr _kdt;
    float _orng, _dfact, _eprop;
    cv::Vec3f _evec, _nvec, _uvec;
    cv::Vec3f _v0, _v1;
    std::string _err;
};  // end class


// Estimate the normal vector using an interative approach that evaluates
// normals along line segments measured over vertical line segments under the
// detected positions of the eyes. The left and right positions v0 and v1 should
// be given by the surface locations of the left and right pupils.
// After obtaining the normal vector, the up vector is calculated simply
// as the normalized vector vnorm.cross(v1-v0)
FaceTools_EXPORT void findOrientation( const RFeatures::ObjModelKDTree::Ptr kdt,
                                       const cv::Vec3f& v0,    // Position of left eye
                                       const cv::Vec3f& v1,    // Position of right eye
                                       cv::Vec3f& vnorm);      // Output face norm
}}   // end namespace

#endif
