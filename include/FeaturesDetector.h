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

#ifndef FACE_TOOLS_FEATURES_DETECTOR_H
#define FACE_TOOLS_FEATURES_DETECTOR_H

#include "FaceTools.h"

#define EYE0_MODEL_FILE  "haarcascades/haarcascade_eye.xml"
#define EYE1_MODEL_FILE  "haarcascades/haarcascade_lefteye_2splits.xml"
#define EYE2_MODEL_FILE  "haarcascades/haarcascade_mcs_lefteye.xml"
#define EYE3_MODEL_FILE  "haarcascades/haarcascade_righteye_2splits.xml"
#define EYE4_MODEL_FILE  "haarcascades/haarcascade_mcs_righteye.xml"
#define FACE0_MODEL_FILE "haarcascades/haarcascade_frontalface_default.xml"
#define FACE1_MODEL_FILE "haarcascades/haarcascade_frontalface_alt.xml"
#define FACE2_MODEL_FILE "haarcascades/haarcascade_frontalface_alt_tree.xml"
#define FACE3_MODEL_FILE "haarcascades/haarcascade_frontalface_alt2.xml"

namespace FaceTools
{

class FaceTools_EXPORT FeaturesDetector
{
public:
    // Initialise the detector by loading the Haar Cascades model files.
    // Returns true IFF files are present and loaded okay.
    static bool init();

    // Applies a contrast stretch to img.
    explicit FeaturesDetector( const cv::Mat_<byte>& img);

    // Try to detect a single face from the given 2D single channel intensity image.
    // Returns true IFF a face is detected (accessed by getFaceBox()).
    bool findFace();
    inline const cv::Rect& getFaceBox() const { return _faceBox;}

    // Search for two eyes within the previously found facebox.
    // (findFace() must be called prior to calling this function).
    // True is returned IFF both the left and right eyes are found.
    // The x,y coordinates of the eye boxes are with respect to the
    // face box returned by getFaceBox().
    bool findEyes();
    inline const cv::Rect& getLeftEye() const { return _leftEyeBox;}
    inline const cv::Rect& getRightEye() const { return _rightEyeBox;}

private:
    static std::vector<RFeatures::HaarCascadeDetector::Ptr> s_faceDetectors;
    static std::vector<RFeatures::HaarCascadeDetector::Ptr> s_eyeDetectors;

    const cv::Mat_<byte> _dimg;    // Image to run detection over for face
    std::vector<RFeatures::HaarCascadeDetector::Ptr> _faceDetectors;
    cv::Mat_<byte> _faceImg;  // Image of face used for eye and nose detection
    std::vector<RFeatures::HaarCascadeDetector::Ptr> _eyeDetectors;
    cv::Rect _faceBox;
    cv::Rect _leftEyeBox;
    cv::Rect _rightEyeBox;

    FeaturesDetector( const FeaturesDetector&);
    void operator=( const FeaturesDetector&);

    void setDetectorsFromFaceBox();
};  // end class

}   // end namespace

#endif

