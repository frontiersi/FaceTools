#ifndef FACE_TOOLS_FACE_FINDER_2D_H
#define FACE_TOOLS_FACE_FINDER_2D_H

#include "FaceTools.h"

namespace FaceTools
{

class FeaturesDetector;

class FaceTools_EXPORT FaceFinder2D
{
public:
    explicit FaceFinder2D( const cv::Mat_<byte> lightMap);

    bool find();    // Looks for face and eyes only

    // Get the absolute positions in the view (as proportions of the view size)
    // of the respective face elements.
    const cv::RotatedRect& getLeftEyeBox() const { return _leye;}
    const cv::RotatedRect& getRightEyeBox() const { return _reye;}

    cv::Point2f getLEyeCentre() const { return _leye.center;}
    cv::Point2f getREyeCentre() const { return _reye.center;}

    void drawDebug( cv::Mat_<cv::Vec3b>& dimg) const;   // Draw debug boxes on given image

private:
    const cv::Mat_<byte> _lightMap;

    // All feature boxes are stored as proportions of the view
    cv::RotatedRect _faceBox;
    cv::RotatedRect _leye, _reye;  // Left and right eye (from viewer's perspective)
    cv::RotatedRect _interPupilSpace;

    bool findEyes( FeaturesDetector*);

    FaceFinder2D( const FaceFinder2D&); // NO COPY
    void operator=( const FaceFinder2D&);   // NO COPY
};  // end class

}   // end namespace

#endif
