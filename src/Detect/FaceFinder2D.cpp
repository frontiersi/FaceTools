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

#include <FaceFinder2D.h>
#include <FeatureUtils.h>       // RFeatures
#include <FeaturesDetector.h>
#include <MiscFunctions.h>
#include <cassert>
using FaceTools::Detect::FaceFinder2D;
using FaceTools::Detect::FeaturesDetector;

namespace {
// Estimate the centre point of the face from the centres of the left and right eyes
cv::Point2f calcFaceCentrePoint( const cv::Point& lc, const cv::Point& rc)
{
    const cv::Vec2f ev( rc.x - lc.x, rc.y - lc.y);  // Eye vector
    assert( ev[0] >= 0);
    // Set the orthogonal vector to be -90 degrees to ev
    // (note here that the opposite indices are negated because OpenCV
    // coordinate origin is the top left - not the bottom left!)
    const cv::Vec2f ov( -ev[1], ev[0]);
    // Calculate the centre point of the space
    const cv::Vec2f cpv = (ev + ov)/2;
    return cv::Point2f( (float)lc.x + cpv[0], (float)lc.y + cpv[1]);
}   // end calcFaceCentrePoint
}   // end namespace


// public
bool FaceFinder2D::find( const cv::Mat_<byte> lightMap)
{
    bool found = false;
    _faceBox = cv::RotatedRect();
    if (!FeaturesDetector::find( lightMap))
        std::cerr << "[WARNING] FaceTools::Detect::FaceFinder2D::find: Unable to find a face!" << std::endl;
    else if ( findEyes( lightMap))
        found = true;
    return found;
}   // end find


// private
bool FaceFinder2D::findEyes( const cv::Mat_<byte> lightMap)
{
    const cv::Size msz = lightMap.size();
    cv::Rect faceBox = FeaturesDetector::faceBox();
    assert( faceBox.area() > 0);

    // Reset
    _leye = cv::RotatedRect();
    _reye = cv::RotatedRect();
    cv::Rect leye = FeaturesDetector::leftEye();
    cv::Rect reye = FeaturesDetector::rightEye();

    // Eye boxes are detected relative to the face, so add the position of the
    // face box to the eye boxes to get their absolute positions.
    leye.x += faceBox.x;
    leye.y += faceBox.y;
    reye.x += faceBox.x;
    reye.y += faceBox.y;

    // Create the faceBox as a rotated rectangle by using the angles between the
    // eye centres to decide the orientation of the face.
    const cv::Point lc = RFeatures::calcPixelCentre( leye);
    const cv::Point rc = RFeatures::calcPixelCentre( reye);
    const cv::Point2f fc = calcFaceCentrePoint( lc, rc);
    const float degAngle = FaceTools::calcAngleDegs( lc, rc);
    const float ipdelta = cv::norm(rc-lc);
    // Make the face width twice the interpupilary distance (height remains the same)
    cv::Size2f fbSize( 2*ipdelta, faceBox.height);
    cv::RotatedRect faceBoxRR( fc, fbSize, degAngle);
    _faceBox = FaceTools::toProportion( faceBoxRR, msz);

    // Rotate the eye boxes too
    _leye = FaceTools::toProportion( cv::RotatedRect( lc, leye.size(), degAngle), msz);
    _reye = FaceTools::toProportion( cv::RotatedRect( rc, reye.size(), degAngle), msz);

    /*
    const cv::RotatedRect interPupilSpace( fc, cv::Size2f(ipdelta, ipdelta), degAngle);
    _interPupilSpace = FaceTools::toProportion( interPupilSpace, msz);
    */
    return true;
}   // end findEyes


// public
cv::Mat_<cv::Vec3b> FaceFinder2D::drawDebug( cv::Mat_<cv::Vec3b> dimg) const
{
    const cv::Size msz = dimg.size();
    const cv::RotatedRect leye = FaceTools::fromProportion( _leye, msz);
    const cv::RotatedRect reye = FaceTools::fromProportion( _reye, msz);
    const cv::RotatedRect faceBox = FaceTools::fromProportion( _faceBox, msz);

    if ( faceBox.size.area())
        RFeatures::drawRotatedRect( dimg, faceBox, 2, cv::Scalar(255,255,255));
    if ( leye.size.area())
        RFeatures::drawRotatedRect( dimg, leye, 2, cv::Scalar(255,50,50));
    if ( reye.size.area())
        RFeatures::drawRotatedRect( dimg, reye, 2, cv::Scalar(50,50,255));
    return dimg;
}   // end drawDebug
