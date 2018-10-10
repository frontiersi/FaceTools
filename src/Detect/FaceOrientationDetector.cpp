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
#include <FaceShapeLandmarks2DDetector.h>
#include <FaceFinder2D.h>
#include <LandmarksManager.h>
#include <DijkstraShortestPathFinder.h> // RFeatures
#include <ObjModelSurfacePointFinder.h> // RFeatures
#include <FeatureUtils.h>               // RFeatures
#include <RendererPicker.h>     // RVTK
#include <ImageGrabber.h>       // RVTK
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


namespace {
/*
cv::Mat_<cv::Vec3b> snapshot( RVTK::Viewer::Ptr viewer, const std::vector<cv::Point2f>& fpts)
{
    cv::Mat_<cv::Vec3b> cmap = snapshot(viewer);
    std::vector<cv::Point> pts;
    std::for_each( std::begin(fpts), std::end(fpts), [&](auto f){ pts.push_back( cv::Point(f.x*cmap.cols, f.y*cmap.rows));});
    RFeatures::drawPoly( pts, cmap, cv::Scalar(255,100,100));
    return cmap;
}   // end snapshot


void showOrientationDebugImage( vtkRenderer* ren, const cv::Vec3f& v0, const cv::Vec3f& v1)
{
    std::vector<cv::Point> pts;
    RVTK::RendererPicker rpicker( ren, RVTK::RendererPicker::TOP_LEFT);
    pts.push_back( rpicker.projectToImagePlane( v0));
    pts.push_back( rpicker.projectToImagePlane( v1));
    RFeatures::drawPoly( pts, dmat, cv::Scalar(100,100,255));
    RFeatures::showImage( dmat, "Detected eye centres", false);
}   // end showOrientationDebugImage
*/

bool pick3DEyes( vtkRenderer* ren, cv::Point2f& f0, cv::Vec3f& v0, cv::Point2f& f1, cv::Vec3f& v1)
{
    RVTK::RendererPicker rpicker( ren, RVTK::RendererPicker::TOP_LEFT);
    cv::Point2f fmid = (f1 + f0) * 0.5f;
    // While the given image positions for the eyes aren't returning an actor, move the points closer
    // in towards the expected position of the nose. Picking points under these image positions can
    // fail if there are holes in the model on the eyes. Small holes over the eyes can happen because
    // some photogrammetric techniques are less robust to highly reflective surfaces.

    // If haven't found valid points in space within MAX_CHECK tries, the model is too full of holes and we give up.
    static const int MAX_CHECK = 20;
    int checkCount = 0;
    while ( !rpicker.pickActor(f0) && checkCount < MAX_CHECK)
    {
        f0.x += 0.005;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return false;

    checkCount = 0;
    while ( !rpicker.pickActor(f1) && checkCount < MAX_CHECK)
    {
        f1.x -= 0.005;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return false;

    checkCount = 0;
    while ( !rpicker.pickActor(fmid) && checkCount < MAX_CHECK)   // Very unlikely to ever be the case
    {
        fmid.y -= 0.005;
        checkCount++;
    }   // end while

    if ( checkCount == MAX_CHECK)
        return false;

    v0 = rpicker.pickWorldPosition( f0);
    v1 = rpicker.pickWorldPosition( f1);

#ifndef NDEBUG
    std::cerr << "Eye detection points:" << std::endl;
    std::cerr << "Left eye:  " << f0   << " --> " << v0 << std::endl;
    std::cerr << "Right eye: " << f1   << " --> " << v1 << std::endl;
#endif
    return true;
}   // end pick3DEyes


bool detect2DEyes( RVTK::Viewer::Ptr viewer, cv::Point2f& f0, cv::Point2f& f1)
{
    FaceFinder2D faceFinder;
    RVTK::ImageGrabber ig(*viewer);
    if ( !faceFinder.find( ig.light()))
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


void updateNormal( const KDT::Ptr kdt, const cv::Vec3f& v0, int e0, const cv::Vec3f& v1, int e1, cv::Vec3f& nvec)
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
void FaceTools::Detect::findOrientation( const KDT::Ptr kdt, const cv::Vec3f& v0, const cv::Vec3f& v1, cv::Vec3f& nvec)
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
}   // end findOrientation


// public
FaceOrientationDetector::FaceOrientationDetector( const KDT::Ptr kdt, float orng, float dfact)
    : _kdt(kdt), _orng(orng), _dfact(orng/dfact), _eprop(0.0f), _nvec(0,0,1), _uvec(0,1,0)
{
    _vwr = FaceTools::makeOffscreenViewer( 400, orng, kdt->model());
}   // end ctor


// public
bool FaceOrientationDetector::orient()
{
    if ( !FD::isinit())
    {
        _err = "Face orientation detector not initialised!";
        return false;
    }   // end if

    // Reset orientation and camera
    _evec = cv::Vec3f(1,0,0);
    _uvec = cv::Vec3f(0,1,0);
    _nvec = cv::Vec3f(0,0,1);
    _vwr->setCamera( RFeatures::CameraParams( _orng * _nvec, cv::Vec3f(0,0,0), _uvec));
    _vwr->updateRender();
    _eprop = 0.0f;
    _err = "";

#ifndef NDEBUG
    RFeatures::showImage( snapshot(_vwr), "Initial pose", false);
#endif

    _eprop = detect3DEyePositionsFrom2D();
    if ( _eprop < 0)
    {
        _err = "Initial face/eye detection failed!";
        return false;
    }   // end if
    setCameraToFace();
#ifndef NDEBUG
    RFeatures::showImage( snapshot(_vwr), "After initial face/eye detection", false);
#endif

    if ( detect3DEyePositionsFrom2D() < 0)
    {
        _err = "Secondary face/eye detection failed!";
        return false;
    }   // end if
    setCameraToFace();
#ifndef NDEBUG
    RFeatures::showImage( snapshot(_vwr), "After secondary face/eye detection", false);
#endif

    findOrientation( _kdt, _v0, _v1, _nvec);
    setCameraToFace();
#ifndef NDEBUG
    RFeatures::showImage( snapshot(_vwr), "After orientation", false);
#endif
    return true;
}   // end orient


bool FaceOrientationDetector::detect( LandmarkSet& lmks)
{
    if ( !_err.empty())
        return false;

    if ( !FLD::isinit())
    {
        _err = "Face landmarks detector not initialised!";
        return false;
    }   // end if

    if ( !FLD::detect( _vwr, &*_kdt, lmks))
    {
        _err = "Detection of landmarks failed!";
        return false;
    }   // end if

    // Update orientation from the discovered landmarks
    _evec = lmks.eyeVec();
    _v0 = *lmks.pos( FaceTools::Landmark::P, FACE_LATERAL_LEFT);
    _v1 = *lmks.pos( FaceTools::Landmark::P, FACE_LATERAL_RIGHT);

    findOrientation( _kdt, _v0, _v1, _nvec);
    setCameraToFace();
#ifndef NDEBUG
    RFeatures::showImage( snapshot(_vwr), "After detection", false);
#endif
    return true;
}   // end detect


// private
float FaceOrientationDetector::detect3DEyePositionsFrom2D()
{
    cv::Point2f f0, f1;
    if ( !detect2DEyes( _vwr, f0, f1))
    {
        std::cerr << "2D eye detection failed!" << std::endl;
        return -1;
    }   // end if
    if ( !pick3DEyes( _vwr->renderer(), f0, _v0, f1, _v1))
    {
        std::cerr << "3D eye picking failed!" << std::endl;
        return -1;
    }   // end if
    _evec = _v1 - _v0;   // Eye vector to right (assumes upright face!)
    return float(cv::norm( f1 - f0));
}   // end detect3DEyePositionsFrom2D


// private
// Standardise the position of the camera looking at the face from a range decided by
// the inter-eye distance of the detected face. This should be run twice because the
// second calculation will be more accurate due to being based on a standardised range
// (and benefitting from a more upright orientation).
void FaceOrientationDetector::setCameraToFace()
{
    // Update camera position for better detection
    const float ediff = float(cv::norm(_evec)); // Distance between eyes
    // Set up vector using current normal vector and eye vector.
    cv::normalize( _nvec.cross( _evec), _uvec);
    // Use adjusted up vector and eye vector to calculate a new normal vector.
    cv::normalize( _evec.cross( _uvec), _nvec);

    const float drng = _eprop * _dfact;
    const cv::Vec3f m = _v0 + 0.5f*_evec;   // Mid-point between eyes
    const cv::Vec3f f = m - 0.2f*ediff*_uvec;   // Focus slightly below eye mid-point
    const cv::Vec3f p = drng*_nvec + m - 0.5f*ediff*_uvec;    // Want to have camera looking up slightly

    // Need to calculate a new up vector because the normal is adjusted to point slightly downward
    cv::normalize( p - f, _nvec);
    cv::normalize( _nvec.cross( _evec), _uvec);

    _vwr->setCamera( RFeatures::CameraParams( p, f, _uvec));
    _vwr->updateRender();
}   // end setCameraToFace
