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
#include <DijkstraShortestPathFinder.h>

using FaceTools::Detect::FaceShapeLandmarks2DDetector;
using FaceTools::Landmark::LandmarkSet;
using RFeatures::ObjModelKDTree;


namespace {

using namespace FaceTools;
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

cv::Vec3f findMSO( const LandmarkSet& lms, cv::Vec3f v, FaceLateral lat)
{
    // Ensure MSO is in vertical with pupil.
    const cv::Vec3f& p = *lms.pos( Landmark::PS, lat);
    v[0] = p[0];
    return v;
}   // end findMSO


cv::Vec3f findAL( const ObjModelKDTree* kdt, const cv::Vec3f& sn, const cv::Vec3f& p)
{
    return FaceTools::findDeepestPoint( kdt, sn, sn + 0.5f*(p-sn));
}   // end findAL


cv::Vec3f findAC( const ObjModelKDTree* kdt, const cv::Vec3f& sn, const cv::Vec3f& al, const cv::Vec3f& p)
{
    cv::Vec3f tv = FaceTools::toSurface( kdt, cv::Vec3f( p[0], sn[1], p[2]));
    return FaceTools::findDeepestPoint( kdt, tv, al);
}   // end findAC


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

    auto toS = FaceTools::toSurface;
    //auto moveTo = FaceTools::moveTo;

    // Left palpebral superius and inferius
    lms.set( Landmark::PS, toS( kdt, 0.5f * (vpts[37] + vpts[38])), FACE_LATERAL_LEFT);
    lms.set( Landmark::PI, toS( kdt, 0.5f * (vpts[40] + vpts[41])), FACE_LATERAL_LEFT);

    cv::Vec3f enw( 0, 0, -25);
    cv::Vec3f exw( 0, 0, -5);

    // Left endo and exo canthi
    cv::Vec3f en = vpts[39] + enw;
    cv::Vec3f ex = vpts[36] + exw;
    en[0] += 3;
    lms.set( Landmark::EN, toS( kdt, en), FACE_LATERAL_LEFT);
    lms.set( Landmark::EX, toS( kdt, ex), FACE_LATERAL_LEFT);

    // Left pupil
    lms.set( Landmark::P, toS( kdt, FaceTools::calcPupil( lms, FACE_LATERAL_LEFT)), FACE_LATERAL_LEFT);
    const cv::Vec3f& pleft = *lms.pos(Landmark::P, FACE_LATERAL_LEFT);

    // Right palpebral superius and inferius
    lms.set( Landmark::PS, toS( kdt, 0.5f * (vpts[43] + vpts[44])), FACE_LATERAL_RIGHT);
    lms.set( Landmark::PI, toS( kdt, 0.5f * (vpts[46] + vpts[47])), FACE_LATERAL_RIGHT);

    // Right endo and exo canthi
    en = vpts[42] + enw;
    ex = vpts[45] + exw;
    en[0] -= 3;
    lms.set( Landmark::EN, toS( kdt, en), FACE_LATERAL_RIGHT);
    lms.set( Landmark::EX, toS( kdt, ex), FACE_LATERAL_RIGHT);

    // Right pupil
    lms.set( Landmark::P, toS( kdt, FaceTools::calcPupil( lms, FACE_LATERAL_RIGHT)), FACE_LATERAL_RIGHT);
    const cv::Vec3f& pright = *lms.pos(Landmark::P, FACE_LATERAL_RIGHT);

    // Mid-supraorbital
    // [17,21] left brow left to right, [22,26] right brow left to right
    lms.set( Landmark::MSO, toS( kdt, findMSO( lms, (1.0f/3) * (vpts[18] + vpts[19] + vpts[20]), FACE_LATERAL_LEFT)), FACE_LATERAL_LEFT);
    lms.set( Landmark::MSO, toS( kdt, findMSO( lms, (1.0f/3) * (vpts[23] + vpts[24] + vpts[25]), FACE_LATERAL_RIGHT)), FACE_LATERAL_RIGHT);

    // Glabella
    lms.set( Landmark::G, toS( kdt, 0.5f * (vpts[21] + vpts[22])));

    // Nasion
    // vpts[27] is nasal root, but not defined as either sellion or nasion so is not used as is.
    cv::Vec3f nv = vpts[27];    // Nasion is placed at height of superoir palpebral sulcus in line with sellion.
    nv[1] = 0.5f * ((*lms.pos( Landmark::PS, FACE_LATERAL_LEFT))[1] + (*lms.pos( Landmark::PS, FACE_LATERAL_RIGHT))[1]);
    lms.set( Landmark::N, toS( kdt, nv));

    // Subnasale
    lms.set( Landmark::SN, toS( kdt, vpts[33]));
    cv::Vec3f sn = *lms.pos(Landmark::SN);   // For use later

    // Pronasale
    cv::Vec3f prn = vpts[30];
    prn[2] += 20;
    lms.set( Landmark::PRN, toS( kdt, prn));

    // Mid-nasal dorsum
    lms.set( Landmark::MND, toS( kdt, findMND( *lms.pos( Landmark::PRN), *lms.pos( Landmark::N))));

    // Sellion - deepest part of the nose bridge.
    // Dynamically weighted in the direction of greatest difference.
    double d0 = 1;
    double d1 = 1;
    cv::Vec3f sev = toS( kdt, FaceTools::findDeepestPoint( kdt, *lms.pos(Landmark::G), *lms.pos(Landmark::MND), &d0));
    cv::Vec3f seh = toS( kdt, FaceTools::findDeepestPoint( kdt, pleft, pright, &d1));
    double dsum = d0 + d1;
    lms.set( Landmark::SE, toS( kdt, (d0/dsum*sev + d1/dsum*seh)));

    // Alare
    //lms.set( Landmark::AL, vpts[31], FACE_LATERAL_LEFT);
    //lms.set( Landmark::AL, vpts[35], FACE_LATERAL_RIGHT);
    lms.set( Landmark::AL, findAL( kdt, sn, pleft), FACE_LATERAL_LEFT);
    lms.set( Landmark::AL, findAL( kdt, sn, pright), FACE_LATERAL_RIGHT);

    // Alare curvature point
    lms.set( Landmark::AC, findAC( kdt, sn, *lms.pos(Landmark::AL, FACE_LATERAL_LEFT), pleft), FACE_LATERAL_LEFT);
    lms.set( Landmark::AC, findAC( kdt, sn, *lms.pos(Landmark::AL, FACE_LATERAL_RIGHT), pright), FACE_LATERAL_RIGHT);

    // Subalare
    //lms.set( Landmark::SBAL, toS( kdt, vpts[32]), FACE_LATERAL_LEFT);
    //lms.set( Landmark::SBAL, toS( kdt, vpts[34]), FACE_LATERAL_RIGHT);
    lms.set( Landmark::SBAL, toS( kdt, 0.5f*(sn + *lms.pos(Landmark::AC, FACE_LATERAL_LEFT))), FACE_LATERAL_LEFT);
    lms.set( Landmark::SBAL, toS( kdt, 0.5f*(sn + *lms.pos(Landmark::AC, FACE_LATERAL_RIGHT))), FACE_LATERAL_RIGHT);

    // Update subnasale
    //lms.set( Landmark::SN, toS( kdt, 0.5f*(*lms.pos(Landmark::SBAL, FACE_LATERAL_LEFT) + *lms.pos(Landmark::SBAL, FACE_LATERAL_RIGHT))));
    //sn = *lms.pos(Landmark::SN);   // For use later

    // Cheilion
    lms.set( Landmark::CH, toS( kdt, vpts[48]), FACE_LATERAL_LEFT);
    lms.set( Landmark::CH, toS( kdt, vpts[54]), FACE_LATERAL_RIGHT);

    // Crista philtri
    lms.set( Landmark::CPH, toS( kdt, vpts[50]), FACE_LATERAL_LEFT);
    lms.set( Landmark::CPH, toS( kdt, vpts[52]), FACE_LATERAL_RIGHT);

    // Labiale superius
    lms.set( Landmark::LS, toS( kdt, vpts[51]));

    // Labiale inferius
    lms.set( Landmark::LI, toS( kdt, 1.0f/3 * (vpts[56] + vpts[57] + vpts[58])));

    // Stomion inferius/superius. Detector can get confused with the placement of these points,
    // so see which one is lower and make inferius.
    const cv::Vec3f s0 = toS( kdt, 1.0f/3 * (vpts[65] + vpts[66] + vpts[67]));
    const cv::Vec3f s1 = toS( kdt, 1.0f/3 * (vpts[61] + vpts[62] + vpts[63]));
    if ( s0[1] < s1[1])
    {
        lms.set( Landmark::STI, s0);
        lms.set( Landmark::STS, s1);
    }   // end if
    else
    {
        lms.set( Landmark::STI, s1);
        lms.set( Landmark::STS, s0);
    }   // end else

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
