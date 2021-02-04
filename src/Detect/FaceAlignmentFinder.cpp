/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Detect/FaceAlignmentFinder.h>
#include <Detect/FaceFinder2D.h>
#include <rimg/FeatureUtils.h>
#include <r3d/VectorPCFinder.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <cmath>
using FaceTools::Detect::FaceAlignmentFinder;
using r3dvis::OffscreenMeshViewer;
using r3d::Vec3f;
using r3d::Mat4f;


bool FaceAlignmentFinder::isInit() { return FaceTools::Detect::FaceFinder2D::isInit();}


namespace {
/*
cv::Mat_<cv::Vec3b> snapshot( const OffscreenMeshViewer& vwr, const std::vector<cv::Point2f>& fpts)
{
    cv::Mat_<cv::Vec3b> cmap = vwr.snapshot();
    std::vector<cv::Point> pts;
    std::for_each( std::begin(fpts), std::end(fpts), [&](auto f){ pts.push_back( cv::Point(f.x*cmap.cols, f.y*cmap.rows));});
    rimg::drawPoly( pts, cmap, cv::Scalar(255,100,100));
    return cmap;
}   // end snapshot


void showOrientationDebugImage( const OffscreenMeshViewer& vwr, const Vec3f& v0, const Vec3f& v1)
{
    std::vector<cv::Point> pts;
    pts.push_back( vwr.imagePlane( v0));
    pts.push_back( vwr.imagePlane( v1));
    rimg::drawPoly( pts, dmat, cv::Scalar(100,100,255));
    rimg::showImage( dmat, "Detected eye centres", false);
}   // end showOrientationDebugImage
*/


bool pick3DEyes( const OffscreenMeshViewer& vwr, cv::Point2f& f0, Vec3f& v0, cv::Point2f& f1, Vec3f& v1)
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


Mat4f estimateTransform( const r3d::KDTree &kdt, const Vec3f &v0, const Vec3f &v1)
{
    // Get vertices from around the eyes
    const float r = (0.7f*(v1 - v0)).squaredNorm();
    std::vector<std::pair<size_t, float> > nearV0;
    kdt.findr( v0, r, nearV0);
    std::vector<std::pair<size_t, float> > nearV1;
    kdt.findr( v1, r, nearV1);

    IntSet vids; // Collate the vertex IDs
    for ( const auto& p : nearV0)
        vids.insert( int(p.first));
    for ( const auto& p : nearV1)
        vids.insert( int(p.first));

    Mat4f T = Mat4f::Identity();
    T.block<3,3>(0,0) = r3d::VectorPCFinder::estimateRotationMatrix( kdt.mesh(), vids);
    const float h = (0.5f*(v1 - v0)).squaredNorm();
    const Vec3f midEyePos = 0.5f*(v0 + v1);
    T.block<3,1>(0,3) = midEyePos - (0.75f*sqrtf(h))*T.block<3,1>(0,1);

    return T;
}   // end estimateTransform


r3d::CameraParams makeCameraParams( const Mat4f T, float crng)
{
    const Vec3f f = T.block<3,1>(0,3);
    const Vec3f p = f + crng * T.block<3,1>(0,2);
    return r3d::CameraParams( p, f, T.block<3,1>(0,1));
}   // end makeCameraParams

}   // end namespace


float FaceAlignmentFinder::_findEyes( Vec3f &v0, Vec3f &v1)
{
    _err = "";
    cv::Mat img = _vwr.lightnessSnapshot();
    //cv::imshow( "_findEyes", img);
    FaceTools::Detect::FaceFinder2D faceFinder;
    if ( !faceFinder.find( img))
    {
        _err = "2D eye detection failed!";
        return -1;
    }   // end if

    cv::Point2f f0 = faceFinder.leyeCentre();
    cv::Point2f f1 = faceFinder.reyeCentre();
    if ( !pick3DEyes( _vwr, f0, v0, f1, v1))
    {
        _err = "3D eye projection failed!";
        return -1;
    }   // end if

    /*
    std::cerr << "Found eyes: " << std::endl;
    std::cerr << "Left at:  " << f0 << " --> " << v0.transpose() << std::endl;
    std::cerr << "Right at: " << f1 << " --> " << v1.transpose() << std::endl;
    */
    return cv::norm(f1 - f0);
}   // end _findEyes


FaceAlignmentFinder::FaceAlignmentFinder() : _vwr( cv::Size(400,400)), _interEyeDist(0.0f) {}


Mat4f FaceAlignmentFinder::find( const r3d::KDTree &kdt, const Vec3f &centre, float orng, float dfact)
{
    _vwr.setModel( kdt.mesh());

    static const int MAX_OTRIES = 10;
    static const int MAX_OALIGN = 4;
    int otries = 0;
    const float ostep = orng / 70;
    dfact = orng/dfact;
    float rng = orng - MAX_OTRIES * ostep;
    float drng = -1;
    bool oriented = false;

    // Intially, the camera should be set to look at the centre of the model bounding box
    // since the model could be located anywhere and we need eye detection to work.
    Mat4f baseT = Mat4f::Identity();
    baseT.block<3,1>(0,3) = centre;
    Mat4f T = baseT;

    Vec3f v0, v1;
    int i = 0;
    while ( i < MAX_OALIGN)  // Use max attempts to align at any particular detection range
    {
        _vwr.setCamera( makeCameraParams( T, rng)); // Set camera to orientation range
        _err = "";
        oriented = false;
        drng = dfact * _findEyes( v0, v1);// (Re)detect 2D eyes and project to 3D as _v0 (left) and _v1 (right)

        if ( drng > 0)
        {
            oriented = true;
            i++;
            T = estimateTransform( kdt, v0, v1);
        }   // end if
        else    // Fail to detect?
        {
            if ( otries >= MAX_OTRIES)
            {
                std::cerr << "[WARNING] FaceTools::Detect::FaceAlignmentFinder::find: " << _err << std::endl;
                _err =  "No fully successful set of face orientations at any range!";
                std::cerr << _err << std::endl;
                break;
            }   // end if

            otries++;
            i = 0;  // Reset range attempts
            T = baseT;
            rng += ostep;  // Increase range for next try
        }   // end else
    }   // end while

    _interEyeDist = oriented ? (v0-v1).norm() : 0.0f;
    return oriented ? T : Mat4f::Zero();
}   // end find
