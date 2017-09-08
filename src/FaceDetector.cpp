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

#include <FaceDetector.h>
using FaceTools::FaceDetector;
using FaceTools::ModelViewer;
using FaceTools::ObjMetaData;

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/serialize.h>
#include <dlib/opencv.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>
#include <ImageGrabber.h>    // RVTK
#include <FaceTools.h>
#include <NoseFinder.h>
#include <FaceFinder2D.h>
using RFeatures::ObjModel;


class FaceDetector::Impl
{
private:
    dlib::frontal_face_detector _faceDetector;
    dlib::shape_predictor _shapePredictor;

public:
    Impl( const std::string& faceShapeLandmarksDat)
        : _faceDetector( dlib::get_frontal_face_detector())
    {
        dlib::deserialize( faceShapeLandmarksDat) >> _shapePredictor;
    }   // end ctor


    bool detectFeatures( const ModelViewer* viewer, ObjMetaData::Ptr omd)
    {
        RVTK::ImageGrabber imgGrabber( viewer->getRenderWindow());
        imgGrabber.update();
        const cv::Mat_<byte> lightMap = imgGrabber.getLightMap();
        int nrows = lightMap.rows;
        int ncols = lightMap.cols;
        const cv::Mat_<cv::Vec3b> colMap = imgGrabber.getColourMap();

        //dlib::cv_image<byte> img(lightMap);
        dlib::cv_image<dlib::bgr_pixel> img(colMap);

        std::vector<dlib::rectangle> dets = _faceDetector( img);
        if ( dets.empty())
            return false;

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

        dlib::full_object_detection det = _shapePredictor( img, dets[j]);
        const int numParts = det.num_parts();
        int nfound = 0;

        std::vector<bool> foundVec(numParts);
        std::vector<cv::Point2f> cpts(numParts);
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

        setLandmarks( viewer, omd, foundVec, cpts);
        return nfound == 68;
    }   // end detectFeatures


    void setLandmarks( const ModelViewer* viewer, ObjMetaData::Ptr omd,
            const std::vector<bool>& foundVec, const std::vector<cv::Point2f>& cpts)
    {
        const int np = (int)foundVec.size();
        std::vector<cv::Vec3f> vpts(np);
        for ( int i = 17; i < np; ++i)  // Vertices before 17 are ignored since these are boundary vertices
        {
            if ( foundVec[i])
                vpts[i] = viewer->project( cpts[i]);
        }   // end for

        using namespace FaceTools::Landmarks;

        omd->setLandmark( L_EYEBROW_0, vpts[17]);
        omd->setLandmark( L_EYEBROW_1, vpts[18]);
        omd->setLandmark( L_EYEBROW_2, vpts[19]);
        omd->setLandmark( L_EYEBROW_3, vpts[20]);
        omd->setLandmark( L_EYEBROW_4, vpts[21]);

        omd->setLandmark( R_EYEBROW_0, vpts[22]);
        omd->setLandmark( R_EYEBROW_1, vpts[23]);
        omd->setLandmark( R_EYEBROW_2, vpts[24]);
        omd->setLandmark( R_EYEBROW_3, vpts[25]);
        omd->setLandmark( R_EYEBROW_4, vpts[26]);

        omd->setLandmark( NASAL_ROOT, vpts[27]);
        // Nasal ridge (28,29) is best defined separately by looking at the shortest path between 27 and 30
        // Nasal tip (30) already defined
        omd->setLandmark( L_ALARE, vpts[31]);
        omd->setLandmark( L_PHILTRUM_T, vpts[32]);
        omd->setLandmark( SUBNASALE, vpts[33]);
        omd->setLandmark( R_PHILTRUM_T, vpts[34]);
        omd->setLandmark( R_ALARE, vpts[35]);

        // LEFT EYE
        omd->setLandmark( L_LAT_CANTH, vpts[36]);
        omd->setLandmark( L_EYELID_T_0, vpts[37]);
        omd->setLandmark( L_EYELID_T_1, vpts[38]);
        omd->setLandmark( L_MED_CANTH, vpts[39]);
        omd->setLandmark( L_EYELID_B_0, vpts[40]);
        omd->setLandmark( L_EYELID_B_1, vpts[41]);

        // LEFT EYE CENTRE
        cv::Vec3f v0(0,0);
        for ( int i = 36; i <= 41; ++i)
            v0 += vpts[i];
        v0 *= 1.0f/6;
        omd->setLandmark( L_EYE_CENTRE, v0);

        // RIGHT EYE
        omd->setLandmark( R_MED_CANTH, vpts[42]);
        omd->setLandmark( R_EYELID_T_0, vpts[43]);
        omd->setLandmark( R_EYELID_T_1, vpts[44]);
        omd->setLandmark( R_LAT_CANTH, vpts[45]);
        omd->setLandmark( R_EYELID_B_0, vpts[46]);
        omd->setLandmark( R_EYELID_B_1, vpts[47]);

        // RIGHT EYE CENTRE
        cv::Vec3f v1(0,0);
        for ( int i = 42; i <= 47; ++i)
            v1 += vpts[i];
        v1 *= 1.0f/6;
        omd->setLandmark( R_EYE_CENTRE, v1);

        // MOUTH
        omd->setLandmark( L_MOUTH_C, vpts[48]);
        omd->setLandmark( L_UPP_VERM, vpts[49]);
        omd->setLandmark( L_PHILTRUM_B, vpts[50]);   // Top left of cupid's bow (upper lip)
        omd->setLandmark( LABIALE_SUP, vpts[51]);    // Top middle of upper lip
        omd->setLandmark( R_PHILTRUM_B, vpts[52]);   // Top right of cupid's bow (upper lip)
        omd->setLandmark( R_UPP_VERM, vpts[53]);
        omd->setLandmark( R_MOUTH_C, vpts[54]);
        omd->setLandmark( LOW_LIP_B_0, vpts[55]);
        omd->setLandmark( LOW_LIP_B_1, vpts[56]);
        omd->setLandmark( LOW_LIP_B_2, vpts[57]);
        omd->setLandmark( LOW_LIP_B_3, vpts[58]);
        omd->setLandmark( LOW_LIP_B_4, vpts[59]);

        // MOUTH OPENING
        omd->setLandmark( MOUTH_OPEN_0, vpts[60]);
        omd->setLandmark( MOUTH_OPEN_1, vpts[61]);
        omd->setLandmark( MOUTH_OPEN_2, vpts[62]);
        omd->setLandmark( MOUTH_OPEN_3, vpts[63]);
        omd->setLandmark( MOUTH_OPEN_4, vpts[64]);
        omd->setLandmark( MOUTH_OPEN_5, vpts[65]);
        omd->setLandmark( MOUTH_OPEN_6, vpts[66]);
        omd->setLandmark( MOUTH_OPEN_7, vpts[67]);

        // VTK's projection of landmarks to model surface is not very accurate, so shift them to be incident with the surface.
        omd->shiftLandmarksToSurface();
    }   // end setLandmarks
};  // end class


class FaceDetector::Deleter
{ public:
    void operator()( FaceDetector* d) { delete d;}
};  // end class


// public static
FaceDetector::Ptr FaceDetector::create( const std::string& haarCascadesModelDir, const std::string& faceShapeLandmarksModel)
{
    FaceTools::FeaturesDetector::Ptr fd = FaceTools::FeaturesDetector::create( haarCascadesModelDir);
    if ( fd == NULL)
    {
        std::cerr << "[ERROR] FaceTools::FaceDetector::create: Unable to create FeaturesDetector using directory " << haarCascadesModelDir << std::endl;
        return Ptr();
    }   // end if

    Impl* impl = new Impl( faceShapeLandmarksModel);
    // TODO make creation of Impl static so this test makes sense
    if ( impl == NULL)
    {
        std::cerr << "[ERROR] FaceTools::FaceDetector::create: Unable to create detector using " << faceShapeLandmarksModel << std::endl;
        return Ptr();
    }   // end if

    return Ptr( new FaceDetector( fd, impl), Deleter());
}   // end create



// private
FaceDetector::FaceDetector( FaceTools::FeaturesDetector::Ptr fd, Impl* impl)
    : _featuresDetector(fd),
      _impl( impl),
      _viewer( new ModelViewer( true/*floodlights*/, true/*offscreen rendering*/))
{
    //_viewer->show();
}   // end ctor


// private
FaceDetector::~FaceDetector()
{
    delete _viewer;
    delete _impl;
}   // end dtor


namespace {
// Detect the initial oriention points for the eyes and nosetip.
bool findOrientationPoints( FaceTools::FeaturesDetector::Ptr fd, ModelViewer& viewer,
                            const ObjModel::Ptr model, cv::Vec3f& v0, cv::Vec3f& v1, cv::Vec3f& ntip)
{
    RVTK::ImageGrabber imgGrabber( viewer.getRenderWindow());
    imgGrabber.update();
    FaceTools::FaceFinder2D faceFinder( fd);
    const cv::Mat_<byte> lightMap = imgGrabber.getLightMap();
    if ( !faceFinder.find( lightMap))
        return false;

    const cv::Mat_<float> dmap = imgGrabber.getDepthMap();
    cv::Point2f f0 = faceFinder.getLEyeCentre();    // [0,1]
    cv::Point2f f1 = faceFinder.getREyeCentre();    // [0,1]
    int count = 0;
    const double eyeDepthSum = RFeatures::findSumBetweenPoints<float>( dmap, f0, f1, 0, FLT_MAX, count);
    const double eyeDepth = eyeDepthSum / count;
    const RFeatures::CameraParams cp = viewer.getCamera();
    // Estimate the 3D positions of the eyes by perspective projection
    v0 = cp.project( cv::Point2f( f0.x - 0.5f, f0.y - 0.5f), eyeDepth); // Left
    v1 = cp.project( cv::Point2f( f1.x - 0.5f, f1.y - 0.5f), eyeDepth); // Right

    // Need a reliable surface to detect the nosetip.
    const cv::Vec3f eyeVec = v1 - v0;
    const cv::Vec3f midEyePoint = v0 + 0.5 * eyeVec;
    ObjModel::Ptr m1 = FaceTools::crop( model, midEyePoint, 70);
    FaceTools::fillHoles( m1);

    // Find the vertices of the clean submodel that are closest to the estimated world positions of the eyes
    RFeatures::ObjModelKDTree::Ptr kdtree = RFeatures::ObjModelKDTree::create(m1);
    const int m1LeftEyeVidx = kdtree->find( v0);
    const int m1RightEyeVidx = kdtree->find( v1);
    RFeatures::ObjModelCurvatureMap::Ptr cmap = RFeatures::ObjModelCurvatureMap::create( m1, m1LeftEyeVidx);
    size_t numSmoothIterations = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( 0.7, numSmoothIterations);
    FaceTools::NoseFinder m1NoseFinder( cmap, m1LeftEyeVidx, m1RightEyeVidx);

    const bool found = m1NoseFinder.find();
    if ( found)
        ntip = m1NoseFinder.getNoseTip();
    return found;
}   // end findOrientationPoints


void resetViewer( ModelViewer *viewer, const ObjModel::Ptr model, bool updateRender=true)
{
    viewer->removeAll();
    viewer->resetDefaultCamera();
    const ModelViewer::VisOptions visOptions( ModelViewer::VisTexture);
    viewer->add( model, visOptions);
    if (updateRender)
        viewer->updateRender();
}   // end resetViewer


// Set 12 boundary landmark vertices
void setBoundary( ObjMetaData::Ptr omd, const std::list<int>& blist)
{
    const ObjModel::Ptr model = omd->getObject();
    std::vector<cv::Vec3f> bvecs;
    BOOST_FOREACH ( int nv, blist)
        bvecs.push_back( model->vtx(nv));
    omd->setBoundary(bvecs);

    // Set the boundary landmarks (handles on the extents of the boundary)
    std::vector<cv::Vec3f> bhandles;
    const bool madeHandles = omd->makeBoundaryHandles( bhandles);
    assert( madeHandles);
    const int nhandles = (int)bhandles.size();
    for ( int i = 0; i < nhandles; ++i)
    {
        std::ostringstream oss;
        oss << FaceTools::Landmarks::BOUNDARY_PRFX << i;
        omd->setLandmark( oss.str(), bhandles[i]);
    }   // end for
}   // setBoundary


void findBoundary( ObjMetaData::Ptr omd, const double G)
{
    using namespace RFeatures;
    using namespace FaceTools;
    const cv::Vec3f& v0 = omd->getLandmark( Landmarks::L_EYE_CENTRE);
    const cv::Vec3f& v1 = omd->getLandmark( Landmarks::R_EYE_CENTRE);
    const cv::Vec3f& nt = omd->getLandmark( Landmarks::NASAL_TIP);

    const ObjModel::Ptr model = omd->getObject();
    const cv::Vec3f fc = FaceTools::calcFaceCentre( omd);

    // Get the list of vertices on the facial boundary G times the distance between the face centre and the eye mid point.
    const cv::Vec3f mp = (v0 + v1) * 0.5f;
    const double brad = G * cv::norm(fc-mp);
    ObjModelCropper cropper( fc, brad);
    ObjModelTriangleMeshParser parser( model);
    parser.setBoundaryParser(&cropper);
    parser.parse( *model->getFaceIds( omd->getKDTree()->find(nt)).begin());

    const std::list<int>& blist = cropper.getBoundary();    // Boundary list of vertices
    setBoundary( omd, blist);
}   // end findBoundary

}   // end namespace


// public
bool FaceDetector::findOrientation( ObjMetaData::Ptr omd)
{
    const ObjModel::Ptr model = omd->getObject();
    if ( model == NULL)
        return false;

    resetViewer( _viewer, model);

    cv::Vec3f v0, v1, ntip;
    if ( !findOrientationPoints( _featuresDetector, *_viewer, model, v0, v1, ntip))
        return false;

    using namespace FaceTools;
    omd->setLandmark( Landmarks::NASAL_TIP, ntip);
    omd->setLandmark( Landmarks::L_EYE_CENTRE, v0);
    omd->setLandmark( Landmarks::R_EYE_CENTRE, v1);

    const cv::Vec3f mp = (v0 + v1 + ntip) * 1.0f/3;
    const double cropRad = 2*std::max(cv::norm(mp-v0), std::max(cv::norm(mp-v1), cv::norm(mp-ntip)));
    ObjModel::Ptr cmodel = FaceTools::crop( model, mp, cropRad);
    FaceTools::fillHoles( cmodel);

    RFeatures::ObjModelKDTree::Ptr kdtree = RFeatures::ObjModelKDTree::create(cmodel);
    RFeatures::ObjModelCurvatureMap::Ptr cmap = RFeatures::ObjModelCurvatureMap::create( cmodel, kdtree->find(ntip));
    size_t numSmoothIterations = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( 0.7, numSmoothIterations);

    FaceOrienter orienter( cmap, kdtree);
    cv::Vec3d nvec, uvec;
    orienter( v0, v1, nvec, uvec);
    omd->setOrientation( nvec, uvec);
    return true;
}   // end findOrientation



// public
bool FaceDetector::findLandmarks( ObjMetaData::Ptr omd)
{
    const ObjModel::Ptr model = omd->getObject();
    if ( model == NULL)
        return false;

    if ( !omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP))
        return false;

    cv::Vec3f nvec, uvec;
    if ( !omd->getOrientation(nvec,uvec))
        return false;

    resetViewer( _viewer, model, false/*don't update render until camera moved*/);
    const cv::Vec3f& ntip = omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);
    _viewer->setCamera( ntip, nvec, uvec, 650);   // Set nose tip as focus
    _viewer->updateRender();

    // Model aligned to maximise good detection of feature points
    const bool found = _impl->detectFeatures( _viewer, omd);
    if ( !found)
        std::cerr << "[WARNING] FaceTools::FaceDetector::findLandmarks: Found only a subset of landmarks!" << std::endl;
    else
        findBoundary( omd, 2.3);

    return found;
}   // end findLandmarks

