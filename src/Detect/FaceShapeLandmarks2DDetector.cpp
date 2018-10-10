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

#include <FaceShapeLandmarks2DDetector.h>
#include <LandmarksManager.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>
#include <VtkTools.h>           // RVTK
#include <ImageGrabber.h>       // RVTK
#include <RendererPicker.h>     // RVTK
#include <FaceTools.h>
#include <MiscFunctions.h>

using FaceTools::Detect::FaceShapeLandmarks2DDetector;
using FaceTools::Landmark::LandmarkSet;
using RFeatures::ObjModelKDTree;


namespace {

/*
void drawDots( cv::Mat& dimg, const std::vector<cv::Point2f>& dots)
{
    const int nrows = dimg.rows - 1;
    const int ncols = dimg.cols - 1;
    for ( const cv::Point2f& p : dots)
    {
        cv::Point pt( p.x * ncols, p.y * nrows);
        cv::line( dimg, pt, pt, CV_RGB(255,255,255), 3);
    }   // end foreach
}   // end drawDots
*/

cv::Vec3f findMND( const cv::Vec3f& pv, const cv::Vec3f& nv) { return pv + 0.5f*(nv - pv);}


// Project detected points to landmarks
void setLandmarks( const RVTK::Viewer::Ptr viewer,
                   const std::vector<bool>& foundVec,
                   const std::vector<cv::Point2f>& cpts,
                   const ObjModelKDTree* kdt,
                   LandmarkSet& lms)
{
    const size_t np = foundVec.size();
    assert( np == cpts.size());
    RVTK::RendererPicker rpicker( viewer->renderer(), RVTK::RendererPicker::TOP_LEFT);
    std::vector<cv::Vec3f> vpts(np);
    // Vertices < 17 ignored since these are boundary vertices and are not accurate.
    for ( size_t i = 17; i < np; ++i)
        vpts[i] = foundVec[i] ? rpicker.pickWorldPosition( cpts[i]) : cv::Vec3f(0,0,0);

    using namespace FaceTools;
    auto toS = FaceTools::toSurface;

    // LEFT EYE
    lms.set( Landmark::PS, toS( kdt, 0.5f * (vpts[37] + vpts[38])), FACE_LATERAL_LEFT);
    lms.set( Landmark::PI, toS( kdt, 0.5f * (vpts[40] + vpts[41])), FACE_LATERAL_LEFT);
    lms.set( Landmark::EN, toS( kdt, vpts[39]), FACE_LATERAL_LEFT);
    lms.set( Landmark::EX, toS( kdt, vpts[36]), FACE_LATERAL_LEFT);
    lms.set( Landmark::P, toS( kdt, FaceTools::calcPupil( lms, FACE_LATERAL_LEFT)), FACE_LATERAL_LEFT);

    // RIGHT EYE
    lms.set( Landmark::PS, toS( kdt, 0.5f * (vpts[43] + vpts[44])), FACE_LATERAL_RIGHT);
    lms.set( Landmark::PI, toS( kdt, 0.5f * (vpts[46] + vpts[47])), FACE_LATERAL_RIGHT);
    lms.set( Landmark::EN, toS( kdt, vpts[42]), FACE_LATERAL_RIGHT);
    lms.set( Landmark::EX, toS( kdt, vpts[45]), FACE_LATERAL_RIGHT);
    lms.set( Landmark::P, toS( kdt, FaceTools::calcPupil( lms, FACE_LATERAL_RIGHT)), FACE_LATERAL_RIGHT);

    // [17,21] left brow left to right, [22,26] right brow left to right
    lms.set( Landmark::MSO, toS( kdt, (1.0f/3) * (vpts[18] + vpts[19] + vpts[20])), FACE_LATERAL_LEFT);    // Mid-supraorbital (left)
    lms.set( Landmark::MSO, toS( kdt, (1.0f/3) * (vpts[23] + vpts[24] + vpts[25])), FACE_LATERAL_RIGHT);   // Mid-supraorbital (right)
    lms.set( Landmark::G, toS( kdt, 0.5f * (vpts[21] + vpts[22])));    // Glabella

    lms.set( Landmark::SE, toS( kdt, vpts[27]));   // Sellion (deepest part)   TODO

    cv::Vec3f nv = vpts[27]; // Nasion - placed at height of superoir palpebral sulcus in line with sellion.
    nv[1] = 0.5f * ((*lms.pos( Landmark::PS, FACE_LATERAL_LEFT))[1] + (*lms.pos( Landmark::PS, FACE_LATERAL_RIGHT))[1]);
    lms.set( Landmark::N, toS( kdt, nv));

    lms.set( Landmark::SN, toS( kdt, vpts[33]));   // Subnasale
    lms.set( Landmark::PRN, toS( kdt, vpts[30]));  // Pronasale TODO use longest restricted path between subnasale and sellion

    // TODO set left/right alare as widest points on the nose
    // Also set subalare (SBAL) properly.
    lms.set( Landmark::AL, vpts[31], FACE_LATERAL_LEFT);
    lms.set( Landmark::AL, vpts[35], FACE_LATERAL_RIGHT);
    lms.set( Landmark::SBAL, toS( kdt, vpts[32]), FACE_LATERAL_LEFT);  // Subalare
    lms.set( Landmark::SBAL, toS( kdt, vpts[34]), FACE_LATERAL_RIGHT); // Subalare

    // MND halway along nasal ridge.
    lms.set( Landmark::MND, toS( kdt, findMND( *lms.pos( Landmark::PRN), *lms.pos( Landmark::N))));

    // Mouth
    lms.set( Landmark::CH, toS( kdt, vpts[48]), FACE_LATERAL_LEFT);     // Cheilion
    lms.set( Landmark::CH, toS( kdt, vpts[54]), FACE_LATERAL_RIGHT);    // Cheilion
    lms.set( Landmark::CPH, toS( kdt, vpts[50]), FACE_LATERAL_LEFT);    // Crista philtri
    lms.set( Landmark::CPH, toS( kdt, vpts[52]), FACE_LATERAL_RIGHT);   // Crista philtri
    lms.set( Landmark::LS, toS( kdt, vpts[51]));   // Labiale superius
    lms.set( Landmark::LI, toS( kdt, 1.0f/3 * (vpts[56] + vpts[57] + vpts[58])));   // Labiale inferius
    lms.set( Landmark::STS, toS( kdt, 1.0f/3 * (vpts[65] + vpts[66] + vpts[67])));   // Stomion superius
    lms.set( Landmark::STI, toS( kdt, 1.0f/3 * (vpts[61] + vpts[62] + vpts[63])));   // Stomion inferius

    //lms.set( L_UPP_VERM, vpts[49]
    //lms.set( R_UPP_VERM, vpts[53]
    //lms.set( LOW_LIP_B_0, vpts[55]
    //lms.set( LOW_LIP_B_4, vpts[59]

    // MOUTH OPENING
    //lms.set( MOUTH_OPEN_0, vpts[60]
    //lms.set( MOUTH_OPEN_4, vpts[64]
}   // end setLandmarks

}   // end namespace


dlib::shape_predictor FaceShapeLandmarks2DDetector::s_shapePredictor;  // static initialisation

// public static
bool FaceShapeLandmarks2DDetector::isinit() { return s_shapePredictor.num_parts() > 0;}


// public static
bool FaceShapeLandmarks2DDetector::initialise( const std::string& fdat)
{
    bool success = false;
    try
    {
        dlib::deserialize( fdat) >> s_shapePredictor;
        success = s_shapePredictor.num_parts() > 0;
    }   // end try
    catch ( const dlib::serialization_error&)
    {
        std::cerr << "[ERROR] FaceTools::Detect::FaceShapeLandmarks2DDetector::initialise: Deserialisation error using \""
                  << fdat << "\"" << std::endl;
        success = false;
    }   // end catch

    return success;
}   // end initialise


// public static
bool FaceShapeLandmarks2DDetector::detect( RVTK::Viewer::Ptr viewer, const ObjModelKDTree* kdt, LandmarkSet& lms)
{
    if ( s_shapePredictor.num_parts() == 0)
    {
        std::cerr << "[ERROR] FaceTools::Detect::FaceShapeLandmarks2DDetector::detect: Not initialised with shape predictor model!"
                  << std::endl;
        return false;
    }   // end if

    const int nrows = viewer->height();
    const int ncols = viewer->width();
    cv::Mat_<cv::Vec3b> map = RVTK::ImageGrabber(*viewer).colour();
    dlib::cv_image<dlib::bgr_pixel> img(map);

    dlib::frontal_face_detector faceDetector( dlib::get_frontal_face_detector());
    std::vector<dlib::rectangle> dets = faceDetector( img);
    if ( dets.empty())
    {
        std::cerr << "[WARNING] FaceTools::Detect::FaceShapeLandmarks2DDetector::detect: "
                  << "dlib::frontal_face_detector failed to detect landmarks!" << std::endl;
        return false;
    }   // end if

    size_t j = 0;
    long bottom = dets[0].bottom();
    // Only want the lowest detection in the set
    for ( size_t i = 1; i < dets.size(); ++i)
    {
        if ( dets[i].bottom() > bottom)
        {
            j = i;
            bottom = dets[i].bottom();
        }   // end if
    }   // end else

    dlib::full_object_detection det = s_shapePredictor( img, dets[j]);
    const size_t numParts = det.num_parts();
    int nfound = 0;

    std::vector<bool> foundVec( numParts);
    std::vector<cv::Point2f> cpts( numParts);
    for ( size_t i = 0; i < numParts; ++i)
    {
        const dlib::point& pt = det.part(i);
        foundVec[i] = pt != dlib::OBJECT_PART_NOT_PRESENT;
        if ( !foundVec[i])
            cpts[i] = cv::Point2f(0,0);
        else
        {
            cpts[i] = cv::Point2f( float(pt.x() + 0.5f)/ncols, float(pt.y() + 0.5f)/nrows);
            nfound++;
        }   // end else
    }   // end for

    setLandmarks( viewer, foundVec, cpts, kdt, lms);
    return nfound == 68;
}   // end detectFeatures
