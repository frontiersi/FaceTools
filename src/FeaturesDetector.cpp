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

#include <FeaturesDetector.h>
using FaceTools::FeaturesDetector;
#include <FeatureUtils.h>   // RFeatures
#include <RectCluster.h>    // RFeatures
#include <Random.h>         // rlib
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>
#include <cassert>

typedef RFeatures::HaarCascadeDetector HCD;
typedef RFeatures::RectCluster::Ptr RC;

std::string createPath( const std::string& pdir, const std::string& fname)
{
    boost::filesystem::path path(pdir);
    path /= fname;
    return path.string();
}   // end createPath


// static public
FeaturesDetector::Ptr FeaturesDetector::create( const std::string& parentDir)
{
    FeaturesDetector* fd = new FeaturesDetector( parentDir);

    // Confirm loads
    bool validLoads = true;
    BOOST_FOREACH ( const HCD::Ptr& hcd, fd->_faceDetectors)
    {
        if ( hcd == NULL)
        {
            validLoads = false;
            break;
        }   // end if
    }   // end foreach

    if ( validLoads)
    {
        BOOST_FOREACH ( const HCD::Ptr& hcd, fd->_eyeDetectors)
        {
            if ( hcd == NULL)
            {
                validLoads = false;
                break;
            }   // end if
        }   // end foreach
    }   // end if

    if ( !validLoads)
    {
        delete fd;
        return Ptr();
    }   // end if

    return Ptr( fd);
}   // end init


// private
FeaturesDetector::FeaturesDetector( const std::string& parentDir)
{
    _faceDetectors.push_back( HCD::create( createPath( parentDir, FACE0_MODEL_FILE)));
    _faceDetectors.push_back( HCD::create( createPath( parentDir, FACE1_MODEL_FILE)));
    _faceDetectors.push_back( HCD::create( createPath( parentDir, FACE2_MODEL_FILE)));
    _faceDetectors.push_back( HCD::create( createPath( parentDir, FACE3_MODEL_FILE)));

    _eyeDetectors.push_back( HCD::create( createPath( parentDir, EYE0_MODEL_FILE)));
    _eyeDetectors.push_back( HCD::create( createPath( parentDir, EYE1_MODEL_FILE)));
    _eyeDetectors.push_back( HCD::create( createPath( parentDir, EYE2_MODEL_FILE)));
    _eyeDetectors.push_back( HCD::create( createPath( parentDir, EYE3_MODEL_FILE)));
    _eyeDetectors.push_back( HCD::create( createPath( parentDir, EYE4_MODEL_FILE)));
}   // end ctor


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


void showDebug( const cv::Mat_<byte>& gimg, const std::string& title,
                const std::vector<RC>& clusters)
{
    rlib::Random rndgen(0);
    cv::Mat_<byte> dimg = gimg.clone();
    BOOST_FOREACH ( const RC& rc, clusters)
    {
        const std::list<cv::Rect>& rects = rc->getRectangles();
        // Paint the rectangles in each cluster with a different colour
        cv::Scalar col( (rndgen.getRandomInt() % (255 - 70)) + 70);
        BOOST_FOREACH ( const cv::Rect& r, rects)
            cv::rectangle( dimg, r, col, 2);
    }   // end foreach
    RFeatures::showImage( dimg, title, false);
}   // end showDebug


bool collectDetections( const std::vector<HCD::Ptr>& hcds, std::list<cv::Rect>& boxes)
{
    BOOST_FOREACH ( const HCD::Ptr& hcd, hcds)
    {
        std::vector<cv::Rect> dts;
        hcd->detect( dts);
        boxes.insert( boxes.end(), dts.begin(), dts.end());
    }   // end foreach
    return !boxes.empty();
}   // end collectEyeDetections


struct RectClusterComparator
{
    bool operator()( const RC& rc0, const RC& rc1) const
    {
        return rc0->calcQuality() > rc1->calcQuality();  // Negated for descending order
    }   // end operator()
};  // end class


void printClusterInfo( const RC& rc, std::ostream& os)
{
    const cv::Point cc = rc->getClusterCentre();
    os << rc->calcDensity() << "[Density]"
       << " x " << rc->calcCompactness() << "[Compactness]"
       << " x " << sqrt(rc->getUnion().area()) << "[Sqrt Union Area]"
       << " = " << rc->calcQuality()
       << " (centre = " << cc << ")"
       << " (#rects = " << rc->getRectangles().size() << ")" << std::endl;
}   // end printClusterInfo


// private
bool FeaturesDetector::findEyes()
{
    if ( !checkFaceBox( _faceBox))
        return false;

    std::list<cv::Rect> eyes;
    if ( !collectDetections( _eyeDetectors, eyes))
        return false;

    // Get the two largest clusters
    std::vector<RC> clusters;
    RFeatures::clusterRects( eyes, 0.5, clusters);
    if ( clusters.size() < 2)   // Need at least two clusters of detections
        return false;
/*
#ifndef NDEBUG
    showDebug( _faceImg, "FeaturesDetector::findEyes()", clusters);
#endif
*/

    std::sort( clusters.begin(), clusters.end(), RectClusterComparator());

    // Get the mean size of a detection in these two clusters
    const cv::Rect_<double> meanBox0 = clusters[0]->getMean();
    const cv::Rect_<double> meanBox1 = clusters[1]->getMean();
    if (( meanBox0 & meanBox1).area() > 0)
        return false;

/*
#ifndef NDEBUG
    std::cerr << std::endl;
    printClusterInfo( clusters[0], std::cerr);
    printClusterInfo( clusters[1], std::cerr);
#endif
*/

    const cv::Rect meanBox( (int)cvRound((meanBox0.x + meanBox1.x)/2),
                            (int)cvRound((meanBox0.y + meanBox1.y)/2),
                            (int)cvRound((meanBox0.width + meanBox1.width)/2),
                            (int)cvRound((meanBox0.height + meanBox1.height)/2));

    // Find the left and right centre points of the eyes
    const cv::Point_<float> cp0 = RFeatures::calcOffset( meanBox0, cv::Point2f(0.5, 0.5));
    const cv::Point_<float> cp1 = RFeatures::calcOffset( meanBox1, cv::Point2f(0.5, 0.5));
    cv::Point lcp( cvRound(cp0.x), cvRound(cp0.y));
    cv::Point rcp( cvRound(cp1.x), cvRound(cp1.y));
    if ( lcp.x >= rcp.x)
    {
        lcp = rcp;
        rcp = cv::Point( cvRound(cp0.x), cvRound(cp0.y));
    }   // end if

    cv::Rect leye( lcp.x - meanBox.width/2, lcp.y - meanBox.height/2, meanBox.width, meanBox.height);
    cv::Rect reye( rcp.x - meanBox.width/2, rcp.y - meanBox.height/2, meanBox.width, meanBox.height);

    _leftEyeBox = leye;
    _rightEyeBox = reye;
    return true;
}   // end findEyes


// private
void FeaturesDetector::setDetectorsFromFaceBox( const cv::Mat_<byte> dimg)
{
    // Only use the top 3/5ths of the face box for the eyes
    cv::Rect topHalf = _faceBox;
    topHalf.height = (int)cvRound(3*((double)(_faceBox.height))/5);
    cv::Mat_<byte> thimg = RFeatures::contrastStretch( dimg( topHalf));
    cv::medianBlur( thimg, thimg, 5);
    _faceImg = dimg(_faceBox);
    BOOST_FOREACH ( HCD::Ptr& hcd, _eyeDetectors)
        hcd->setImage( thimg);
}   // end setDetectorsFromFaceBox


// public
bool FeaturesDetector::find( const cv::Mat_<byte> img)
{
    const cv::Mat_<byte> dimg = RFeatures::contrastStretch( img);
    //cv::GaussianBlur( dimg, dimg, cv::Size(9,9), 0);
    //cv::medianBlur( dimg, dimg, 9);
    // Create member instances
    BOOST_FOREACH ( HCD::Ptr& hcd, _faceDetectors)
        hcd->setImage(dimg);

    _faceBox = cv::Rect(0,0,0,0);
    std::list<cv::Rect> faces;
    if ( !collectDetections( _faceDetectors, faces))
        return false;

    std::vector<RC> clusters;
    RFeatures::clusterRects( faces, 0.7f, clusters);
    if ( clusters.empty())
        return false;
/*
#ifndef NDEBUG
    showDebug( dimg, "FACES", clusters);
#endif
*/
    std::sort( clusters.begin(), clusters.end(), RectClusterComparator());

    const cv::Rect_<double> fb = clusters[0]->getMean();
    _faceBox.x = cvRound(fb.x);
    _faceBox.y = cvRound(fb.y);
    _faceBox.width = cvRound(fb.width);
    _faceBox.height = cvRound(fb.height);

    setDetectorsFromFaceBox( dimg);
    return findEyes();
}   // end find
