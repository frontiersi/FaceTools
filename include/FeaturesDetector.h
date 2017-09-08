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

#include "FaceTools_Export.h"
#include <HaarCascadeDetector.h>
#include <boost/shared_ptr.hpp>

#define FACE0_MODEL_FILE "haarcascade_frontalface_default.xml"
#define FACE1_MODEL_FILE "haarcascade_frontalface_alt.xml"
#define FACE2_MODEL_FILE "haarcascade_frontalface_alt_tree.xml"
#define FACE3_MODEL_FILE "haarcascade_frontalface_alt2.xml"
#define EYE0_MODEL_FILE  "haarcascade_eye.xml"
#define EYE1_MODEL_FILE  "haarcascade_lefteye_2splits.xml"
#define EYE2_MODEL_FILE  "haarcascade_mcs_lefteye.xml"
#define EYE3_MODEL_FILE  "haarcascade_righteye_2splits.xml"
#define EYE4_MODEL_FILE  "haarcascade_mcs_righteye.xml"

typedef unsigned char byte;

namespace FaceTools
{

class FaceTools_EXPORT FeaturesDetector
{
public:
    typedef boost::shared_ptr<FeaturesDetector> Ptr;

    // Initialise the detector by loading the Haar Cascades model files
    // from the given directory. Returns true iff files are loaded okay.
    static Ptr create( const std::string& modelDir);

    // Try to detect a single face from the given 2D single channel intensity image.
    // Returns true IFF a face is detected (accessed by getFaceBox()).
    bool find( const cv::Mat_<byte> img);

    inline const cv::Rect& getFaceBox() const { return _faceBox;}
    inline const cv::Rect& getLeftEye() const { return _leftEyeBox;}
    inline const cv::Rect& getRightEye() const { return _rightEyeBox;}

private:
    std::vector<RFeatures::HaarCascadeDetector::Ptr> _faceDetectors;
    cv::Mat_<byte> _faceImg;  // Image of face used for eye and nose detection
    std::vector<RFeatures::HaarCascadeDetector::Ptr> _eyeDetectors;
    cv::Rect _faceBox;
    cv::Rect _leftEyeBox;
    cv::Rect _rightEyeBox;

    void setDetectorsFromFaceBox( const cv::Mat_<byte>);

    // Search for two eyes within the previously found facebox.
    // (findFace() must be called prior to calling this function).
    // True is returned IFF both the left and right eyes are found.
    // The x,y coordinates of the eye boxes are with respect to the
    // face box returned by getFaceBox().
    bool findEyes();

    explicit FeaturesDetector( const std::string&);
    FeaturesDetector( const FeaturesDetector&);
    void operator=( const FeaturesDetector&);
};  // end class

}   // end namespace

#endif

