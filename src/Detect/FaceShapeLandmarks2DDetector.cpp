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

#include <FaceShapeLandmarks2DDetector.h>

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
using FaceTools::LandmarkSet;


namespace {

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


// Project detected points to landmarks
void setLandmarks( const RVTK::Viewer::Ptr viewer,
                   const std::vector<bool>& foundVec,
                   const std::vector<cv::Point2f>& cpts,
                   LandmarkSet::Ptr lset)
{
    const int np = (int)foundVec.size();
    assert( np == (int)cpts.size());
    RVTK::RendererPicker rpicker( viewer->getRenderer(), RVTK::RendererPicker::TOP_LEFT);
    std::vector<cv::Vec3f> vpts(np);
    // Vertices < 17 ignored since these are boundary vertices
    for ( int i = 17; i < np; ++i)
        vpts[i] = foundVec[i] ? rpicker.pickWorldPosition( cpts[i]) : cv::Vec3f(0,0,0);

    std::vector<int> lmids; // For collecting all the newly created landmark IDs

    using namespace FaceTools::Landmarks;
    lmids.push_back( lset->set( L_EYEBROW_0, vpts[17]));
    lmids.push_back( lset->set( L_EYEBROW_1, vpts[18]));
    lmids.push_back( lset->set( L_EYEBROW_2, vpts[19]));
    lmids.push_back( lset->set( L_EYEBROW_3, vpts[20]));
    lmids.push_back( lset->set( L_EYEBROW_4, vpts[21]));

    lmids.push_back( lset->set( R_EYEBROW_0, vpts[22]));
    lmids.push_back( lset->set( R_EYEBROW_1, vpts[23]));
    lmids.push_back( lset->set( R_EYEBROW_2, vpts[24]));
    lmids.push_back( lset->set( R_EYEBROW_3, vpts[25]));
    lmids.push_back( lset->set( R_EYEBROW_4, vpts[26]));

    lmids.push_back( lset->set( NASAL_ROOT, vpts[27]));
    // Nasal ridge (28,29) is best defined separately by looking at the shortest path between 27 and 30
    //lmids.push_back( lset->set( NASAL_TIP, vpts[30])); // Nasal tip (30) already defined
    lmids.push_back( lset->set( L_ALARE, vpts[31]));
    lmids.push_back( lset->set( L_PHILTRUM_T, vpts[32]));
    lmids.push_back( lset->set( SUBNASALE, vpts[33]));
    lmids.push_back( lset->set( R_PHILTRUM_T, vpts[34]));
    lmids.push_back( lset->set( R_ALARE, vpts[35]));

    // LEFT EYE
    lmids.push_back( lset->set( L_LAT_CANTH, vpts[36]));
    lmids.push_back( lset->set( L_EYELID_T_0, vpts[37]));
    lmids.push_back( lset->set( L_EYELID_T_1, vpts[38]));
    lmids.push_back( lset->set( L_MED_CANTH, vpts[39]));
    lmids.push_back( lset->set( L_EYELID_B_0, vpts[40]));
    lmids.push_back( lset->set( L_EYELID_B_1, vpts[41]));

    // LEFT EYE CENTRE
    cv::Vec3f v0(0,0);
    for ( int i = 36; i <= 41; ++i)
        v0 += vpts[i];
    v0 *= 1.0f/6;
    lmids.push_back( lset->set( L_EYE_CENTRE, v0));

    // RIGHT EYE
    lmids.push_back( lset->set( R_MED_CANTH, vpts[42]));
    lmids.push_back( lset->set( R_EYELID_T_0, vpts[43]));
    lmids.push_back( lset->set( R_EYELID_T_1, vpts[44]));
    lmids.push_back( lset->set( R_LAT_CANTH, vpts[45]));
    lmids.push_back( lset->set( R_EYELID_B_0, vpts[46]));
    lmids.push_back( lset->set( R_EYELID_B_1, vpts[47]));

    // RIGHT EYE CENTRE
    cv::Vec3f v1(0,0);
    for ( int i = 42; i <= 47; ++i)
        v1 += vpts[i];
    v1 *= 1.0f/6;
    lmids.push_back( lset->set( R_EYE_CENTRE, v1));

    // MOUTH
    lmids.push_back( lset->set( L_MOUTH_C, vpts[48]));
    lmids.push_back( lset->set( L_UPP_VERM, vpts[49]));
    lmids.push_back( lset->set( L_PHILTRUM_B, vpts[50]));   // Top left of cupid's bow (upper lip)
    lmids.push_back( lset->set( LABIALE_SUP, vpts[51]));    // Top middle of upper lip
    lmids.push_back( lset->set( R_PHILTRUM_B, vpts[52]));   // Top right of cupid's bow (upper lip)
    lmids.push_back( lset->set( R_UPP_VERM, vpts[53]));
    lmids.push_back( lset->set( R_MOUTH_C, vpts[54]));
    lmids.push_back( lset->set( LOW_LIP_B_0, vpts[55]));
    lmids.push_back( lset->set( LOW_LIP_B_1, vpts[56]));
    lmids.push_back( lset->set( LOW_LIP_B_2, vpts[57]));
    lmids.push_back( lset->set( LOW_LIP_B_3, vpts[58]));
    lmids.push_back( lset->set( LOW_LIP_B_4, vpts[59]));

    // MOUTH OPENING
    lmids.push_back( lset->set( MOUTH_OPEN_0, vpts[60]));
    lmids.push_back( lset->set( MOUTH_OPEN_1, vpts[61]));
    lmids.push_back( lset->set( MOUTH_OPEN_2, vpts[62]));
    lmids.push_back( lset->set( MOUTH_OPEN_3, vpts[63]));
    lmids.push_back( lset->set( MOUTH_OPEN_4, vpts[64]));
    lmids.push_back( lset->set( MOUTH_OPEN_5, vpts[65]));
    lmids.push_back( lset->set( MOUTH_OPEN_6, vpts[66]));
    lmids.push_back( lset->set( MOUTH_OPEN_7, vpts[67]));

    // Set all these newly added landmarks to be non-deletable by default
    std::for_each( std::begin(lmids), std::end(lmids), [&]( int i){ lset->get(i)->deletable = false;});
}   // end setLandmarks

}   // end namespace


dlib::shape_predictor FaceShapeLandmarks2DDetector::s_shapePredictor;  // static initialisation


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
bool FaceShapeLandmarks2DDetector::detect( RVTK::Viewer::Ptr viewer, LandmarkSet::Ptr lset)
{
    if ( s_shapePredictor.num_parts() == 0)
    {
        std::cerr << "[ERROR] FaceTools::Detect::FaceShapeLandmarks2DDetector::detect: Not initialised with shape predictor model!"
                  << std::endl;
        return false;
    }   // end if

    const int nrows = viewer->getHeight();
    const int ncols = viewer->getWidth();
    cv::Mat_<cv::Vec3b> map = RVTK::ImageGrabber(viewer).colour();
    dlib::cv_image<dlib::bgr_pixel> img(map);

    dlib::frontal_face_detector faceDetector( dlib::get_frontal_face_detector());
    std::vector<dlib::rectangle> dets = faceDetector( img);
    if ( dets.empty())
    {
        std::cerr << "[WARNING] FaceTools::Detect::FaceShapeLandmarks2DDetector::detect: "
                  << "dlib::frontal_face_detector failed to detect landmarks!" << std::endl;
        return false;
    }   // end if

    int j = 0;
    int bottom = dets[0].bottom();
    // Only want the lowest detection in the set
    for ( int i = 1; i < dets.size(); ++i)
    {
        if ( dets[i].bottom() > bottom)
        {
            j = i;
            bottom = dets[i].bottom();
        }   // end if
    }   // end else

    dlib::full_object_detection det = s_shapePredictor( img, dets[j]);
    const int numParts = det.num_parts();
    int nfound = 0;

    std::vector<bool> foundVec( numParts);
    std::vector<cv::Point2f> cpts( numParts);
    for ( int i = 0; i < numParts; ++i)
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

    setLandmarks( viewer, foundVec, cpts, lset);
    return nfound == 68;
}   // end detectFeatures


// public static
size_t FaceShapeLandmarks2DDetector::numDetectionLandmarks( const LandmarkSet::Ptr lset)
{
    size_t ncount = 0;
    using namespace FaceTools::Landmarks;

    if ( lset->has( L_EYEBROW_0)) ncount++;
    if ( lset->has( L_EYEBROW_1)) ncount++;
    if ( lset->has( L_EYEBROW_2)) ncount++;
    if ( lset->has( L_EYEBROW_3)) ncount++;
    if ( lset->has( L_EYEBROW_4)) ncount++;

    if ( lset->has( R_EYEBROW_0)) ncount++;
    if ( lset->has( R_EYEBROW_1)) ncount++;
    if ( lset->has( R_EYEBROW_2)) ncount++;
    if ( lset->has( R_EYEBROW_3)) ncount++;
    if ( lset->has( R_EYEBROW_4)) ncount++;

    if ( lset->has( L_LAT_CANTH)) ncount++;
    if ( lset->has( L_EYELID_T_0)) ncount++;
    if ( lset->has( L_EYELID_T_1)) ncount++;
    if ( lset->has( L_MED_CANTH)) ncount++;
    if ( lset->has( L_EYELID_B_0)) ncount++;
    if ( lset->has( L_EYELID_B_1)) ncount++;
    if ( lset->has( L_EYE_CENTRE)) ncount++;

    if ( lset->has( R_LAT_CANTH)) ncount++;
    if ( lset->has( R_EYELID_T_0)) ncount++;
    if ( lset->has( R_EYELID_T_1)) ncount++;
    if ( lset->has( R_MED_CANTH)) ncount++;
    if ( lset->has( R_EYELID_B_0)) ncount++;
    if ( lset->has( R_EYELID_B_1)) ncount++;
    if ( lset->has( R_EYE_CENTRE)) ncount++;

    if ( lset->has( NASAL_ROOT)) ncount++;
    if ( lset->has( NASAL_TIP)) ncount++;
    if ( lset->has( L_ALARE)) ncount++;
    if ( lset->has( L_PHILTRUM_T)) ncount++;
    if ( lset->has( SUBNASALE)) ncount++;
    if ( lset->has( R_PHILTRUM_T)) ncount++;
    if ( lset->has( R_ALARE)) ncount++;

    if ( lset->has( L_MOUTH_C)) ncount++;
    if ( lset->has( L_UPP_VERM)) ncount++;
    if ( lset->has( L_PHILTRUM_B)) ncount++;
    if ( lset->has( LABIALE_SUP)) ncount++;
    if ( lset->has( R_PHILTRUM_B)) ncount++;
    if ( lset->has( R_UPP_VERM)) ncount++;
    if ( lset->has( R_MOUTH_C)) ncount++;

    if ( lset->has( LOW_LIP_B_0)) ncount++;
    if ( lset->has( LOW_LIP_B_1)) ncount++;
    if ( lset->has( LOW_LIP_B_2)) ncount++;
    if ( lset->has( LOW_LIP_B_3)) ncount++;
    if ( lset->has( LOW_LIP_B_4)) ncount++;

    if ( lset->has( MOUTH_OPEN_0)) ncount++;
    if ( lset->has( MOUTH_OPEN_1)) ncount++;
    if ( lset->has( MOUTH_OPEN_2)) ncount++;
    if ( lset->has( MOUTH_OPEN_3)) ncount++;
    if ( lset->has( MOUTH_OPEN_4)) ncount++;
    if ( lset->has( MOUTH_OPEN_5)) ncount++;
    if ( lset->has( MOUTH_OPEN_6)) ncount++;
    if ( lset->has( MOUTH_OPEN_7)) ncount++;

    return ncount;
}   // end numDetectionLandmarks
