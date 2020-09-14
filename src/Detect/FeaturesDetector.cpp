/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Detect/FeaturesDetector.h>
#include <rimg/FeatureUtils.h>
#include <rimg/RectCluster.h>
#include <rlib/Random.h>
#include <algorithm>
#include <boost/filesystem/path.hpp>
#include <cassert>
using FaceTools::Detect::FeaturesDetector;
using HCD = rimg::HaarCascadeDetector;
using RC = rimg::RectCluster::Ptr;
using byte = unsigned char;

namespace {

std::string createPath( const std::string& pdir, const std::string& fname)
{
    boost::filesystem::path path(pdir);
    path /= fname;
    return path.string();
}   // end createPath


bool checkFaceBox( const cv::Rect& faceBox)
{
    assert( faceBox.area());
    if ( !faceBox.area())
    {
        std::cerr << "Error FaceDetector: faceBox must have a positive area!" << std::endl;
        return false;
    }   // end if
    return true;
}   // end checkFaceBox

/*
void showDebug( const cv::Mat_<byte>& gimg, const std::string& title,
                const std::vector<RC>& clusters)
{
    rlib::Random rndgen(0);
    cv::Mat_<byte> dimg = gimg.clone();
    for ( const RC& rc : clusters)
    {
        const std::list<cv::Rect>& rects = rc->getRectangles();
        // Paint the rectangles in each cluster with a different colour
        cv::Scalar col( (rndgen.getRandomInt() % (255 - 70)) + 70);
        for ( const cv::Rect& r : rects)
            cv::rectangle( dimg, r, col, 2);
    }   // end foreach
    rimg::showImage( dimg, title, false);
}   // end showDebug
*/


bool collectDetections( const std::vector<HCD::Ptr>& hcds, std::list<cv::Rect>& boxes)
{
    for ( const HCD::Ptr& hcd : hcds)
    {
        std::vector<cv::Rect> dts;
        hcd->detect( dts);
        boxes.insert( boxes.end(), dts.begin(), dts.end());
    }   // end foreach
    return !boxes.empty();
}   // end collectEyeDetections

}   // end namespace


// static initialisers
std::vector<HCD::Ptr> FeaturesDetector::s_faceDetectors;
std::vector<HCD::Ptr> FeaturesDetector::s_eyeDetectors;
cv::Rect FeaturesDetector::s_faceBox;
cv::Rect FeaturesDetector::s_lEyeBox;
cv::Rect FeaturesDetector::s_rEyeBox;


// static public
bool FeaturesDetector::initialise( const std::string& pdir)
{
    s_faceDetectors.clear();
    s_eyeDetectors.clear();

    s_faceDetectors.push_back( HCD::create( createPath( pdir, FACE0_MODEL_FILE)));
    s_faceDetectors.push_back( HCD::create( createPath( pdir, FACE1_MODEL_FILE)));
    s_faceDetectors.push_back( HCD::create( createPath( pdir, FACE2_MODEL_FILE)));
    s_faceDetectors.push_back( HCD::create( createPath( pdir, FACE3_MODEL_FILE)));

    s_eyeDetectors.push_back( HCD::create( createPath( pdir, EYE0_MODEL_FILE)));
    s_eyeDetectors.push_back( HCD::create( createPath( pdir, EYE1_MODEL_FILE)));
    s_eyeDetectors.push_back( HCD::create( createPath( pdir, EYE2_MODEL_FILE)));
    s_eyeDetectors.push_back( HCD::create( createPath( pdir, EYE3_MODEL_FILE)));

    for ( const HCD::Ptr hcd : s_faceDetectors)
    {
        if ( hcd == nullptr)
        {
            s_faceDetectors.clear();
            s_eyeDetectors.clear();
            return false;
        }   // end if
    }   // end for

    for ( const HCD::Ptr hcd : s_eyeDetectors)
    {
        if ( hcd == nullptr)
        {
            s_faceDetectors.clear();
            s_eyeDetectors.clear();
            return false;
        }   // end if
    }   // end for

    return true;
}   // end initialise


// private static
bool FeaturesDetector::findEyes()
{
    std::list<cv::Rect> eyes;
    if ( !collectDetections( s_eyeDetectors, eyes))
        return false;

    // Get the two largest clusters
    std::vector<RC> clusters;
    rimg::clusterRects( eyes, 0.5, clusters);
    if ( clusters.size() < 2)   // Need at least two clusters of detections
        return false;

    // Negated for descending order
    std::sort( clusters.begin(), clusters.end(), []( const RC& rc0, const RC& rc1)
                                                 { return rc0->calcQuality() > rc1->calcQuality();});

    // Get the mean size of a detection in these two clusters
    const cv::Rect_<double> meanBox0 = clusters[0]->getMean();
    const cv::Rect_<double> meanBox1 = clusters[1]->getMean();
    if (( meanBox0 & meanBox1).area() > 0)
        return false;

    const cv::Rect meanBox( (int)cvRound((meanBox0.x + meanBox1.x)/2),
                            (int)cvRound((meanBox0.y + meanBox1.y)/2),
                            (int)cvRound((meanBox0.width + meanBox1.width)/2),
                            (int)cvRound((meanBox0.height + meanBox1.height)/2));

    // Find the left and right centre points of the eyes
    const cv::Point_<float> cp0 = rimg::calcOffset( meanBox0, cv::Point2f(0.5, 0.5));
    const cv::Point_<float> cp1 = rimg::calcOffset( meanBox1, cv::Point2f(0.5, 0.5));
    cv::Point lcp( cvRound(cp0.x), cvRound(cp0.y));
    cv::Point rcp( cvRound(cp1.x), cvRound(cp1.y));
    if ( lcp.x >= rcp.x)
    {
        lcp = rcp;
        rcp = cv::Point( cvRound(cp0.x), cvRound(cp0.y));
    }   // end if

    s_lEyeBox = cv::Rect( lcp.x - meanBox.width/2, lcp.y - meanBox.height/2, meanBox.width, meanBox.height);
    s_rEyeBox = cv::Rect( rcp.x - meanBox.width/2, rcp.y - meanBox.height/2, meanBox.width, meanBox.height);
    return true;
}   // end findEyes


// public static
bool FeaturesDetector::find( const cv::Mat_<byte> img)
{
    const cv::Mat_<byte> dimg = rimg::contrastStretch( img);
    // Create member instances
    std::for_each( s_faceDetectors.begin(), s_faceDetectors.end(), [=]( HCD::Ptr hcd){ hcd->setImage(dimg);});

    s_faceBox = cv::Rect(0,0,0,0);
    std::list<cv::Rect> faces;
    if ( !collectDetections( s_faceDetectors, faces))
        return false;

    std::vector<RC> clusters;
    rimg::clusterRects( faces, 0.7f, clusters);
    if ( clusters.empty())
        return false;

    // Negated for descending order
    std::sort( clusters.begin(), clusters.end(), []( const RC& rc0, const RC& rc1)
                                                 { return rc0->calcQuality() > rc1->calcQuality();});

    const cv::Rect_<double> fb = clusters[0]->getMean();
    s_faceBox.x = cvRound(fb.x);
    s_faceBox.y = cvRound(fb.y);
    s_faceBox.width = cvRound(fb.width);
    s_faceBox.height = cvRound(fb.height);

    //Set detectors from face box
    // Only use the top 3/5ths of the face box for the eyes
    cv::Rect topHalf = s_faceBox;
    topHalf.height = (int)cvRound(3*((double)(s_faceBox.height))/5);
    cv::Mat_<byte> thimg = rimg::contrastStretch( dimg( topHalf));
    cv::medianBlur( thimg, thimg, 5);
    std::for_each( s_eyeDetectors.begin(), s_eyeDetectors.end(), [=]( HCD::Ptr hcd){ hcd->setImage( thimg);});

    if ( !checkFaceBox( s_faceBox))
        return false;

    return findEyes();
}   // end find
