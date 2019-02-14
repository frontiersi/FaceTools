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
using RVTK::OffscreenModelViewer;
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

auto toS = FaceTools::toSurface;
auto toT = FaceTools::toTarget;
auto toD = FaceTools::findDeepestPoint;


cv::Vec3f findMSO( const ObjModelKDTree* kdt, cv::Vec3f v, const cv::Vec3f& p)
{
    float x = p[0]; // In line with pupil
    float y = 0.5f * (p[1] + v[1]); // Halfway between pupil and reference vertex
    float z = v[2]; // Depth of reference vertex
    cv::Vec3f t0 = toS( kdt, cv::Vec3f( x,          y, z));    // Bottom
    cv::Vec3f t1 = toS( kdt, cv::Vec3f( x, 2*v[1] - y, z));    // Top
    return toD( kdt, t0, t1, nullptr);
}   // end findMSO


void setLandmarksPair( const QString& ch, const cv::Vec3f& v0, const cv::Vec3f& v1, LandmarkSet& lms, const IntSet& ulmks)
{
    if ( ulmks.count( LDMKS_MAN::landmark(ch)->id()) > 0)
    {
        lms.set( ch, v0, FACE_LATERAL_LEFT);
        lms.set( ch, v1, FACE_LATERAL_RIGHT);
    }   // end if
}   // setLandmarksPair


void setLandmark( const QString& ch, const cv::Vec3f& v, LandmarkSet& lms, const IntSet& ulmks)
{
    if ( ulmks.count( LDMKS_MAN::landmark(ch)->id()) > 0)
        lms.set( ch, v);
}   // setLandmark


// Project detected points to landmarks
void setLandmarks( const OffscreenModelViewer& vwr,
                   const std::vector<bool>& foundVec,
                   const std::vector<cv::Point2f>& cpts,
                   const ObjModelKDTree* kdt,
                   LandmarkSet& lms,
                   const IntSet& ulmks)
{
    const size_t np = foundVec.size();
    assert( np == cpts.size());
    std::vector<cv::Vec3f> vpts(np);
    // Vertices < 17 ignored since these are boundary vertices and are not accurate.
    for ( size_t i = 17; i < np; ++i)
        vpts[i] = foundVec[i] ? vwr.worldPosition( cpts[i]) : cv::Vec3f(0,0,0);

    // Left and right palpebral superius
    std::cerr << " * Detecting [PS] L" << std::endl;
    cv::Vec3f lps = toS( kdt, 0.5f * (vpts[37] + vpts[38]));
    std::cerr << " * Detecting [PS] R" << std::endl;
    cv::Vec3f rps = toS( kdt, 0.5f * (vpts[43] + vpts[44]));

    // Left and right palpebral inferius
    std::cerr << " * Detecting [PI] L" << std::endl;
    cv::Vec3f lpi = toS( kdt, 0.5f * (vpts[40] + vpts[41]));
    std::cerr << " * Detecting [PI] R" << std::endl;
    cv::Vec3f rpi = toS( kdt, 0.5f * (vpts[46] + vpts[47]));

    cv::Vec3f enw( 0, 0, -25);
    cv::Vec3f exw( 0, 0, -5);

    // Left endo and exo canthi
    std::cerr << " * Detecting [EN] L" << std::endl;
    std::cerr << " * Detecting [EX] L" << std::endl;
    cv::Vec3f len = vpts[39] + enw;
    cv::Vec3f lex = vpts[36] + exw;
    len[0] += 3;
    len = toS( kdt, len);
    lex = toS( kdt, lex);

    // Right endo and exo canthi
    std::cerr << " * Detecting [EN] R" << std::endl;
    std::cerr << " * Detecting [EX] R" << std::endl;
    cv::Vec3f ren = vpts[42] + enw;
    cv::Vec3f rex = vpts[45] + exw;
    ren[0] -= 3;
    ren = toS( kdt, ren);
    rex = toS( kdt, rex);

    // Left pupil
    std::cerr << " * Detecting [P] L" << std::endl;
    const cv::Vec3f lp = toS( kdt, 0.25f * (len + lex + lps + lpi));

    // Right pupil
    std::cerr << " * Detecting [P] R" << std::endl;
    const cv::Vec3f rp = toS( kdt, 0.25f * (ren + rex + rps + rpi));

    // Sellion - deepest part of the nose bridge between the pupils
    std::cerr << " * Detecting [SE]" << std::endl;
    const cv::Vec3f se = toS( kdt, toD( kdt, lp, rp, nullptr));

    // Mid-supraorbital
    // [17,21] left brow left to right, [22,26] right brow left to right
    std::cerr << " * Detecting [MSO] L" << std::endl;
    cv::Vec3f lmso = findMSO( kdt, (1.0f/3) * (vpts[18] + vpts[19] + vpts[20]), lp);    // Left
    std::cerr << " * Detecting [MSO] R" << std::endl;
    cv::Vec3f rmso = findMSO( kdt, (1.0f/3) * (vpts[23] + vpts[24] + vpts[25]), rp);    // Right

    // Glabella
    std::cerr << " * Detecting [G]" << std::endl;
    cv::Vec3f g = 0.5f * (lmso + rmso);
    g[0] = se[0];
    g = toS( kdt, g);
    cv::Vec3f tmp = se;  // Place temp point in line with sellion halfway in y between palpebral superius and mso points
    tmp[1] = 0.25f * (lps[1] + rps[1] + lmso[1] + rmso[1]);
    tmp = toS( kdt, tmp);
    // Find glabella as maximally off curve point between tmp and tmp point placed above halfway (x) between mso points
    g = toD( kdt, tmp, toS( kdt, 2*g - tmp), nullptr);

    // Nasion
    std::cerr << " * Detecting [N]" << std::endl;
    // vpts[27] is nasal root, but not defined as either sellion or nasion so is not used as is.
    cv::Vec3f n = vpts[27];    // Nasion is placed at height of superoir palpebral sulcus in line with sellion.
    /*
    n[0] = se[0];
    n[1] = std::max( 0.5f * (lps[1] + rps[1]), se[1]);
    n = toS( kdt, n);
    n[1] = std::max(n[1], se[1]); // Ensure nasion remains no lower on face than sellion
    */
    n = toD( kdt, g, se, nullptr);

    // Pronasale as on detected nose tip but high in z axis.
    std::cerr << " * Detecting [PRN]" << std::endl;
    cv::Vec3f prn = toS( kdt, vpts[30]);
    prn = toT( kdt, prn, cv::Vec3f(prn[0], prn[1], prn[2] + 100));

    // Mid-nasal dorsum as halfway between sellion and pronasale and high in z axis.
    std::cerr << " * Detecting [MND]" << std::endl;
    cv::Vec3f mnd = toS( kdt, se + 0.5f*( cv::Vec3f( prn[0], prn[1] + 5, prn[2] + 10) - se));

    // Maxillofrontale
    std::cerr << " * Detecting [MF] L" << std::endl;
    cv::Vec3f lref0 = toS( kdt, 0.5f * (g + len));
    cv::Vec3f lref1 = toS( kdt, 0.5f * (mnd + len));
    const cv::Vec3f lmf = toD( kdt, lref0, lref1, nullptr);
    std::cerr << " * Detecting [MF] R" << std::endl;
    cv::Vec3f rref0 = toS( kdt, 0.5f * (g + ren));
    cv::Vec3f rref1 = toS( kdt, 0.5f * (mnd + ren));
    const cv::Vec3f rmf = toD( kdt, rref0, rref1, nullptr);

    // Subnasale
    std::cerr << " * Detecting [SN]" << std::endl;
    cv::Vec3f sn = toS( kdt, vpts[33]);

    // Alare (vpts[31] and vpts[35] left and right respectively)
    std::cerr << " * Detecting [AL] L" << std::endl;
    cv::Vec3f lal = toS( kdt, vpts[31]);
    std::cerr << " * Detecting [AL] R" << std::endl;
    cv::Vec3f ral = toS( kdt, vpts[35]);

    // Crista philtri
    std::cerr << " * Detecting [CPH] L" << std::endl;
    cv::Vec3f lcph = toS( kdt, vpts[50]);
    std::cerr << " * Detecting [CPH] R" << std::endl;
    cv::Vec3f rcph = toS( kdt, vpts[52]);

    // Subalare (32 and 34 left and right)
    std::cerr << " * Detecting [SBAL] L" << std::endl;
    cv::Vec3f lsbal = toS( kdt, vpts[32]);
    std::cerr << " * Detecting [SBAL] R" << std::endl;
    cv::Vec3f rsbal = toS( kdt, vpts[34]);

    cv::Vec3f lac, rac;

    for ( int i = 0; i < 1; ++i)
    {
        std::cerr << " * Updating  [SN]" << std::endl;
        sn = toD( kdt, lsbal, rsbal, nullptr);

        // Find reference points on alare as highest points between subnasale and halfway to pupils.
        std::cerr << " * Updating  [AL] L" << std::endl;
        cv::Vec3f lt = toD( kdt, sn, sn + 0.5f*(lp - sn), nullptr);
        std::cerr << " * Updating  [AL] R" << std::endl;
        cv::Vec3f rt = toD( kdt, sn, sn + 0.5f*(rp - sn), nullptr);

        // Alare curvature point points
        std::cerr << " * Updating  [AC] L" << std::endl;
        lac = toD( kdt, lt, toS( kdt, cv::Vec3f( lp[0], sn[1], 0.5f* (lp[2] + sn[2]))), nullptr);
        std::cerr << " * Updatint  [AC] R" << std::endl;
        rac = toD( kdt, rt, toS( kdt, cv::Vec3f( rp[0], sn[1], 0.5f* (rp[2] + sn[2]))), nullptr);

        // Now update alare points using the just calculated curvature points and the reference points.
        cv::Vec3f lt2 = toS( kdt, lac + 2*(lt - lac));
        lal = toD( kdt, lac, lt2, nullptr);
        cv::Vec3f rt2 = toS( kdt, rac + 2*(rt - rac));
        ral = toD( kdt, rac, rt2, nullptr);

        // Update subalare
        std::cerr << " * Updating  [SBAL] L" << std::endl;
        lsbal = toD( kdt, lal, lcph, nullptr);
        std::cerr << " * Updating  [SBAL] R" << std::endl;
        rsbal = toD( kdt, ral, rcph, nullptr);

        std::cerr << " * Updating  [SN]" << std::endl;
        sn = toD( kdt, lsbal, rsbal, nullptr);
    }   // end for

    // Cheilion
    std::cerr << " * Detecting [CH] L" << std::endl;
    const cv::Vec3f lch = toS( kdt, vpts[48]);
    std::cerr << " * Detecting [CH] R" << std::endl;
    const cv::Vec3f rch = toS( kdt, vpts[54]);

    // Labiale superius
    std::cerr << " * Detecting [LS]" << std::endl;
    const cv::Vec3f ls = toS( kdt, vpts[51]);

    // Labiale inferius
    std::cerr << " * Detecting [LI]" << std::endl;
    const cv::Vec3f li = toS( kdt, 1.0f/3 * (vpts[56] + vpts[57] + vpts[58]));

    // Sublabiale
    std::cerr << " * Detecting [SL]" << std::endl;
    const float hmw = static_cast<float>(cv::norm(lch - rch)/2);   // Half mouth width
    const cv::Vec3f sl = toD( kdt, li, toS(kdt, cv::Vec3f(li[0], li[1]-hmw, li[2])), nullptr);

    // Stomion inferius/superius
    std::cerr << " * Detecting [STS]" << std::endl;
    cv::Vec3f sts = toS( kdt, 1.0f/3 * (vpts[65] + vpts[66] + vpts[67]));
    std::cerr << " * Detecting [STI]" << std::endl;
    cv::Vec3f sti = toS( kdt, 1.0f/3 * (vpts[61] + vpts[62] + vpts[63]));
    // Detector can confuse placement of stomion inferius/superius, so check relative heights.
    if ( sts[1] < sti[1])
        std::swap( sts, sti);

    //lms.set( L_UPP_VERM, vpts[49]
    //lms.set( R_UPP_VERM, vpts[53]
    //lms.set( LOW_LIP_B_0, vpts[55]
    //lms.set( LOW_LIP_B_4, vpts[59]

    // MOUTH OPENING
    //lms.set( MOUTH_OPEN_0, vpts[60]
    //lms.set( MOUTH_OPEN_4, vpts[64]

    setLandmarksPair( Landmark::PI,   lpi,   rpi,   lms, ulmks);
    setLandmarksPair( Landmark::PS,   lps,   rps,   lms, ulmks);
    setLandmarksPair( Landmark::EN,   len,   ren,   lms, ulmks);
    setLandmarksPair( Landmark::EX,   lex,   rex,   lms, ulmks);
    setLandmarksPair( Landmark::MSO,  lmso,  rmso,  lms, ulmks);
    setLandmarksPair( Landmark::P,    lp,    rp,    lms, ulmks);
    setLandmarksPair( Landmark::AC,   lac,   rac,   lms, ulmks);
    setLandmarksPair( Landmark::AL,   lal,   ral,   lms, ulmks);
    setLandmarksPair( Landmark::CPH,  lcph,  rcph,  lms, ulmks);
    setLandmarksPair( Landmark::SBAL, lsbal, rsbal, lms, ulmks);
    setLandmarksPair( Landmark::CH,   lch,   rch,   lms, ulmks);
    setLandmarksPair( Landmark::MF,   lmf,   rmf,   lms, ulmks);

    setLandmark( Landmark::SE,  se,  lms, ulmks);
    setLandmark( Landmark::G,   g,   lms, ulmks);
    setLandmark( Landmark::N,   n,   lms, ulmks);
    setLandmark( Landmark::PRN, prn, lms, ulmks);
    setLandmark( Landmark::MND, mnd, lms, ulmks);
    setLandmark( Landmark::SN,  sn,  lms, ulmks);
    setLandmark( Landmark::LS,  ls,  lms, ulmks);
    setLandmark( Landmark::LI,  li,  lms, ulmks);
    setLandmark( Landmark::SL,  sl,  lms, ulmks);
    setLandmark( Landmark::STS, sts, lms, ulmks);
    setLandmark( Landmark::STI, sti, lms, ulmks);
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
bool FaceShapeLandmarks2DDetector::detect( const OffscreenModelViewer& vwr, const ObjModelKDTree* kdt, LandmarkSet& lms, const IntSet& ulmks)
{
    if ( s_shapePredictor.num_parts() == 0)
    {
        std::cerr << "[ERROR] FaceTools::Detect::FaceShapeLandmarks2DDetector::detect: Not initialised with shape predictor model!"
                  << std::endl;
        return false;
    }   // end if

    cv::Mat_<cv::Vec3b> map = vwr.snapshot();
    const int nrows = map.rows;
    const int ncols = map.cols;

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

    setLandmarks( vwr, foundVec, cpts, kdt, lms, ulmks);
    return nfound == 68;
}   // end detectFeatures
