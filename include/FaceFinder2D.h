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
