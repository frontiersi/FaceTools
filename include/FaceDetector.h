#ifndef FACE_TOOLS_FACE_DETECTOR_H
#define FACE_TOOLS_FACE_DETECTOR_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "Landmarks.h"
#include "ObjMetaData.h"
#include "ModelViewer.h"
#include "FaceTools_Export.h"


namespace FaceTools
{

class FaceTools_EXPORT FaceDetector
{
public:
    explicit FaceDetector( const std::string& faceShapeLandmarksDat);
    ~FaceDetector();

    int findLandmarks( ObjMetaData::Ptr);

private:
    ModelViewer *_viewer;   // Offscreen scene rendering
    struct Impl;
    Impl *_impl;

    FaceDetector( const FaceDetector&);     // NO COPY
    void operator=( const FaceDetector&);   // NO COPY
};  // end class


}   // end namespace

#endif
