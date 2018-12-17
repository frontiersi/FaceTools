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

#include <FaceOrientationDetector.h>
#include <FaceFinder2D.h>
#include <LandmarksManager.h>
#include <DijkstraShortestPathFinder.h> // RFeatures
#include <ObjModelSurfacePointFinder.h> // RFeatures
#include <FeatureUtils.h>               // RFeatures
#include <algorithm>
#include <iostream>
#include <cassert>
#include <cmath>
using KDT = RFeatures::ObjModelKDTree;
using PathFinder = RFeatures::DijkstraShortestPathFinder;
using RFeatures::ObjModel;
using RFeatures::Orientation;
using FaceTools::Detect::FaceFinder2D;
using FaceTools::Landmark::LandmarkSet;
using FD = FaceTools::Detect::FeaturesDetector;
using FaceTools::Detect::FaceOrientationDetector;
using FLD = FaceTools::Detect::FaceShapeLandmarks2DDetector;
using RVTK::OffscreenModelViewer;


namespace {
/*
cv::Mat_<cv::Vec3b> snapshot( const OffscreenModelViewer& vwr, const std::vector<cv::Point2f>& fpts)
{
    cv::Mat_<cv::Vec3b> cmap = vwr.snapshot();
    std::vector<cv::Point> pts;
    std::for_each( std::begin(fpts), std::end(fpts), [&](auto f){ pts.push_back( cv::Point(f.x*cmap.cols, f.y*cmap.rows));});
    RFeatures::drawPoly( pts, cmap, cv::Scalar(255,100,100));
    return cmap;
}   // end snapshot


void showOrientationDebugImage( const OffscreenModelViewer& vwr, const cv::Vec3f& v0, const cv::Vec3f& v1)
{
    std::vector<cv::Point> pts;
    pts.push_back( vwr.imagePlane( v0));
    pts.push_back( vwr.imagePlane( v1));
    RFeatures::drawPoly( pts, dmat, cv::Scalar(100,100,255));
    RFeatures::showImage( dmat, "Detected eye centres", false);
}   // end showOrientationDebugImage
*/

bool pick3DEyes( const OffscreenModelViewer& vwr, cv::Point2f& f0, cv::Vec3f& v0, cv::Point2f& f1, cv::Vec3f& v1)
{
    cv::Point2f fmid = (f1 + f0) * 0.5f;
    // While the given image positions for the eyes aren't returning an actor, move the points closer
    // in towards the expected position of the nose. Picking points under these image positions can
    // fail if there are holes in the model on the eyes. Small holes over the eyes can happen because
    // some photogrammetric techniques are less robust to highly reflective surfaces.

    // If haven't found valid points in space within MAX_CHECK tries, the model is too full of holes and we give up.
    static const int MAX_CHECK = 20;
    int checkCount = 0;
    while ( !vwr.pick(f0) && checkCount < MAX_CHECK)
    {
        f0.x += 0.005f;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return false;

    checkCount = 0;
    while ( !vwr.pick(f1) && checkCount < MAX_CHECK)
    {
        f1.x -= 0.005f;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return false;

    checkCount = 0;
    while ( !vwr.pick(fmid) && checkCount < MAX_CHECK)   // Very unlikely to ever be the case
    {
        fmid.y -= 0.005f;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return false;

    v0 = vwr.worldPosition( f0);
    v1 = vwr.worldPosition( f1);

    /*
#ifndef NDEBUG
    std::cerr << "Eye detection points:" << std::endl;
    std::cerr << "Left eye:  " << f0   << " --> " << v0 << std::endl;
    std::cerr << "Right eye: " << f1   << " --> " << v1 << std::endl;
#endif
    */
    return true;
}   // end pick3DEyes


bool detect2DEyes( const cv::Mat_<byte>& img, cv::Point2f& f0, cv::Point2f& f1)
{
    FaceFinder2D faceFinder;
    if ( !faceFinder.find( img))
        return false;
    f0 = faceFinder.leyeCentre();    // [0,1]
    f1 = faceFinder.reyeCentre();    // [0,1]
    return true;
}   // end detect2DEyes


cv::Vec3f calcMeanNormalBetweenPoints( const ObjModel* model, int v0, int v1)
{
    PathFinder pfinder( model);
    pfinder.setEndPointVertexIndices( v0, v1);
    std::vector<int> vidxs;
    pfinder.findShortestPath( vidxs);
    const int n = int(vidxs.size()) - 1;
    cv::Vec3f nrm(0,0,0);
    for ( int i = 0; i < n; ++i)
    {
        const IntSet& sfids = model->getSharedFaces( vidxs[size_t(i)], vidxs[size_t(i+1)]);
        std::for_each( std::begin(sfids), std::end(sfids), [&](int fid){ nrm += model->calcFaceNorm(fid);});
    }   // end for
    cv::normalize( nrm, nrm);
    return nrm;
}   // end calcMeanNormalBetweenPoints


void updateNormal( const KDT* kdt, const cv::Vec3f& v0, int e0, const cv::Vec3f& v1, int e1, cv::Vec3f& nvec)
{
    const ObjModel* model = kdt->model();

    // Estimate "down" vector from cross product of base vector with current (inaccurate) face normal.
    const cv::Vec3f evec = v1 - v0;
    cv::Vec3f dvec;
    cv::normalize( evec.cross(nvec), dvec);

    // Find reference locations further down the face from e0 and e1
    const float pdelta = float(1.0 * cv::norm(evec));
    const int r0 = kdt->find( v0 + dvec * pdelta);
    const int r1 = kdt->find( v1 + dvec * pdelta);

    // The final view vector is defined as the mean normal along the path over
    // the model between the provided points and the shifted points.
    const cv::Vec3f vv0 = calcMeanNormalBetweenPoints( model, r0, e0);
    const cv::Vec3f vv1 = calcMeanNormalBetweenPoints( model, r1, e1);
    cv::normalize( vv0 + vv1, nvec);
}   // end updateNormal

}   // end namespace


// public
void FaceTools::Detect::findNormal( const KDT* kdt, const cv::Vec3f& v0, const cv::Vec3f& v1, cv::Vec3f& nvec)
{
    const int e0 = kdt->find( v0);
    const int e1 = kdt->find( v1);
    static const double MIN_DELTA = 1e-8;
    static const int MAX_TRIES = 12;

    double delta = MIN_DELTA + 1;
    int tries = 0;
    while ( fabs(delta) > MIN_DELTA && tries < MAX_TRIES)
    {
        const cv::Vec3f invec = nvec;
        updateNormal( kdt, v0, e0, v1, e1, nvec);
        delta = cv::norm( nvec - invec);
        tries++;
    }   // end while
}   // end findNormal


// public
FaceOrientationDetector::FaceOrientationDetector( const KDT* kdt, float orng, float dfact)
    : _vwr( cv::Size(400,400), orng), _kdt(kdt), _orng(orng), _dfact(orng/dfact), _nvec(0,0,1)
{
    _vwr.setModel(kdt->model());
}   // end ctor


// private
float FaceOrientationDetector::orient()
{
    cv::Point2f f0, f1;

    cv::Mat img = _vwr.lightnessSnapshot();
    if ( !detect2DEyes( img, f0, f1))
    {
        _err = "2D eye detection failed!";
        return -1;
    }   // end if

    if ( !pick3DEyes( _vwr, f0, _v0, f1, _v1))
    {
        _err = "3D eye picking failed!";
        return -1;
    }   // end if

    const float eprop = static_cast<float>(cv::norm( f1 - f0));
    return eprop * _dfact; // Detection range
}   // end orient



bool FaceOrientationDetector::detect( LandmarkSet& lmks)
{
    const std::string errhead = "[WARNING] FaceTools::Detect::FaceOrientationDetector::detect: ";
    if ( !FD::isinit() || !FLD::isinit())
    {
        _err = "Face detection not initialised!";
        std::cerr << errhead << _err << std::endl;
        return false;
    }   // end if

    if ( FaceTools::Landmark::LandmarksManager::count() == 0)
    {
        _err = "Landmark data not loaded!";
        std::cerr << errhead << _err << std::endl;
        return false;
    }   // end if

    // Reset orientation and camera
    _nvec = cv::Vec3f(0,0,1);
    _v0 = _v1 = cv::Vec3f(0,0,0);

    // Saved params for detection
    cv::Vec3f sv0, sv1, snvec;
    float sdrng = -1;

    static const int MAX_OTRIES = 10;
    static const int MAX_OALIGN = 4;
    int otries = 0;
    const float ostep = _orng / 70;
    float orng = _orng - MAX_OTRIES * ostep;
    float drng = orng;

    int i = 0;
    while ( i < MAX_OALIGN)  // Use 4 attempts to align at any particular detection range
    {
        std::cerr << "Detecting face (orientation) at range " << orng << std::endl;
        setCameraToFace( orng);    // Set camera to orientation range

        /*
        std::ostringstream oss;
        oss << "Pre-orientation " << i << " (" << orng << ")";
        RFeatures::showImage( _vwr.snapshot(), oss.str(), true);
        */

        _err = "";
        drng = orient();
        if ( drng > 0)
        {
            sdrng = drng;
            if ( i == 0)
            {
                sv0 = _v0;
                sv1 = _v1;
                snvec = _nvec;
            }   // end if
            else
            {
                const float sfact = 1.0f/(i+1);
                sv0 = sfact * (i*sv0 + _v0);
                sv1 = sfact * (i*sv1 + _v1);
                snvec = sfact * (i*snvec + _nvec);
            }   // end else

            i++;
            if ( i < MAX_OALIGN)    // Find a different normal to try a better alignment for eye detection and orientation
                findNormal( _kdt, _v0, _v1, _nvec);
        }   // end if
        else
        {
            if ( otries >= MAX_OTRIES)
            {
                std::cerr << errhead << _err << std::endl;
                std::cerr << "No fully successful set of face orientations at any range!" << std::endl;
                break;
            }   // end if
            else
            {
                otries++;
                orng += ostep;
                i = 0;  // Reset range attempts
                // Reset camera parameters
                _nvec = cv::Vec3f(0,0,1);
                _v0 = _v1 = cv::Vec3f(0,0,0);
            }   // end else
        }   // end if
    }   // end while

    bool detected = false;
    if ( sdrng > 0)
    {
        // Restore the saved parameters
        _v0 = sv0;
        _v1 = sv1;
        _nvec = snvec;

        std::cerr << "Landmark detection range set to " << sdrng << std::endl;
        setCameraToFace( sdrng); // Set camera to detection range
        //RFeatures::showImage( _vwr.snapshot(), "Pre-detection", true);

        if ( !FLD::detect( _vwr, &*_kdt, lmks))
        {
            _err = "Landmark detection failed!";
            std::cerr << errhead << _err << std::endl;
        }   // end if
        else
        {
            detected = true;
            // Update orientation from the discovered landmarks
            _v0 = *lmks.pos( FaceTools::Landmark::P, FACE_LATERAL_LEFT);
            _v1 = *lmks.pos( FaceTools::Landmark::P, FACE_LATERAL_RIGHT);

            findNormal( _kdt, _v0, _v1, _nvec);
            cv::normalize( snvec + _nvec, _nvec);
            /*
            setCameraToFace( sdrng);
            RFeatures::showImage( _vwr.snapshot(), "Adjusted post-detection", true);
            */
        }   // end else
    }   // end if

    return detected;
}   // end detect


// private
// Standardise the position of the camera looking at the face from a range decided by
// the inter-eye distance of the detected face. This should be run twice because the
// second calculation will be more accurate due to being based on a standardised range
// (and benefitting from a more upright orientation).
void FaceOrientationDetector::setCameraToFace( float crng)
{
    cv::Vec3f evec = _v1 - _v0;
    // Update camera position for better detection
    const float ediff = float(cv::norm(evec)); // Distance between eyes
    if ( ediff <= 0)
        evec = cv::Vec3f(1,0,0);

    // Set up vector using current normal vector and eye vector.
    cv::Vec3f uv;
    cv::normalize( _nvec.cross( evec), uv);
    // Use adjusted up vector and eye vector to calculate a new normal vector.
    cv::normalize( evec.cross( uv), _nvec);

    const cv::Vec3f m = 0.5f * (_v0 + _v1);                 // Mid-point between eyes
    const cv::Vec3f f = m - 0.2f*ediff*uv;                          // Focus slightly below eye mid-point
    const cv::Vec3f p = crng*_nvec + m - 0.5f*ediff*uv;  // Want to have camera looking up slightly

    // Need to calculate a new up vector because the normal is adjusted to point slightly downward
    cv::normalize( p - f, _nvec);
    cv::normalize( _nvec.cross( evec), uv);

    _vwr.setCamera( RFeatures::CameraParams( p, f, uv));
}   // end setCameraToFace


RFeatures::Orientation FaceOrientationDetector::orientation() const
{
    const cv::Vec3f evec = (_v1 == _v0) ? cv::Vec3f(1,0,0) : _v1 - _v0;
    cv::Vec3f uv;
    cv::normalize( _nvec.cross( evec), uv);
    return RFeatures::Orientation(_nvec, uv);
}   // end orientation
