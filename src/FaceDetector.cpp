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
#include <ImageGrabber.h>       // RVTK
#include <VtkActorCreator.h>    // RVTK
#include <RendererPicker.h>     // RVTK
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


    bool detectFeatures( RVTK::Viewer::Ptr viewer, ObjMetaData::Ptr omd)
    {
        RVTK::ImageGrabber imgGrabber( viewer->getRenderWindow());
        imgGrabber.update();
        const cv::Mat_<byte> lightMap = imgGrabber.getLightMap();
        //RFeatures::showImage( lightMap, "detectFeatures", true);
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


    void setLandmarks( RVTK::Viewer::Ptr viewer, ObjMetaData::Ptr omd,
                       const std::vector<bool>& foundVec, const std::vector<cv::Point2f>& cpts)
    {
        const int np = (int)foundVec.size();
        assert( np == (int)cpts.size());
        RVTK::RendererPicker rpicker( viewer->getRenderer(), RVTK::RendererPicker::TOP_LEFT);
        std::vector<cv::Vec3f> vpts(np);
        // Vertices before 17 are ignored since these are boundary vertices
        for ( int i = 17; i < np; ++i)
            vpts[i] = foundVec[i] ? rpicker.pickWorldPosition( cpts[i]) : cv::Vec3f(0,0,0);

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
      _viewer( RVTK::Viewer::create(true/*offscreen*/))
{
    _viewer->setSize(512,512);
    _viewer->setClippingRange(0.1,1000);
    // Set flood lights in the viewer
    std::vector<RVTK::Light> lights;
    RVTK::createBoxLights( 600, lights, true);
    RVTK::resetLights( _viewer->getRenderer(), lights);
}   // end ctor


// private
FaceDetector::~FaceDetector()
{
    delete _impl;
}   // end dtor


namespace {

bool getEyePoints( vtkRenderer* ren, cv::Point2f& f0, cv::Vec3f& v0,
                                     cv::Point2f& f1, cv::Vec3f& v1,
                                                      cv::Vec3f& mp)
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
    mp = rpicker.pickWorldPosition( fmid);
    //std::cerr << "v0: " << v0 << std::endl;
    //std::cerr << "v1: " << v1 << std::endl;
    //std::cerr << "mp: " << mp << std::endl;
    return true;
}   // end getEyePoints


// Detect the initial oriention points for the eyes and nosetip.
std::string findOrientationPoints( FaceTools::FeaturesDetector::Ptr fd, RVTK::Viewer::Ptr viewer,
                                   const ObjModel::Ptr model, cv::Vec3f& v0, cv::Vec3f& v1, cv::Vec3f& ntip)
{
    FaceTools::FaceFinder2D faceFinder( fd);
    RVTK::ImageGrabber imgGrabber( viewer->getRenderWindow());
    imgGrabber.update();
    const cv::Mat_<byte> lightMap = imgGrabber.getLightMap();
    //RFeatures::showImage( lightMap, "findOrientationPoints", true);
    if ( !faceFinder.find( lightMap))
        return "Failed to find face from 2D image.";

    cv::Point2f f0 = faceFinder.getLEyeCentre();    // [0,1]
    cv::Point2f f1 = faceFinder.getREyeCentre();    // [0,1]
    // Get the points in space for the eyes. If model is too full of holes, fail.
    cv::Vec3f mp;
    if ( !getEyePoints( viewer->getRenderer(), f0, v0, f1, v1, mp))
        return "Unable to pick model positions from 2D points.";

    // If model is too small around the crop point, fail.
    ObjModel::Ptr m1 = FaceTools::crop( model, mp, 70);
    if ( m1->getNumFaces() < 50)
        return "Cropped model around point mid-eye point has < 50 polygons.";

    FaceTools::fillHoles( m1);

    // Find the vertices of the clean submodel that are closest to the estimated world positions of the eyes
    RFeatures::ObjModelKDTree::Ptr kdtree = RFeatures::ObjModelKDTree::create(m1);
    const int m1LeftEyeVidx = kdtree->find( v0);
    const int m1RightEyeVidx = kdtree->find( v1);
    RFeatures::ObjModelCurvatureMap::Ptr cmap = RFeatures::ObjModelCurvatureMap::create( m1, m1LeftEyeVidx);
    size_t numSmoothIterations = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( 0.7, numSmoothIterations);
    FaceTools::NoseFinder m1NoseFinder( cmap, m1LeftEyeVidx, m1RightEyeVidx);

    if ( m1NoseFinder.find())
        ntip = m1NoseFinder.getNoseTip();
    else
        return "Unable to find nose-tip.";
    return "";
}   // end findOrientationPoints


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
bool FaceDetector::detect( ObjMetaData::Ptr omd)
{
    _err = "";
    const ObjModel::Ptr model = omd->getObject();
    if ( model == NULL)
    {
        _err = "Internal model missing from ObjMetaData!";
        return false;
    }   // end if

    _viewer->clear();   // Clear viewer of existing actors
    RVTK::VtkActorCreator actorCreator;
    std::vector<vtkSmartPointer<vtkActor> > actors;
    actorCreator.generateTexturedActors( model, actors);
    BOOST_FOREACH ( vtkSmartPointer<vtkActor> actor, actors)
        _viewer->addActor(actor);

    if (!findOrientation(omd))
        return false;

    if (!findLandmarks(omd))
        return false;

    return true;
}   // end detect


// private
bool FaceDetector::findOrientation( ObjMetaData::Ptr omd)
{
    const RFeatures::CameraParams dcam( cv::Vec3f(0,0,650));    // Default camera
    _viewer->setCamera( dcam);
    _viewer->updateRender();

    const ObjModel::Ptr model = omd->getObject();
    cv::Vec3f v0, v1, ntip;
    _err = findOrientationPoints( _featuresDetector, _viewer, model, v0, v1, ntip);
    if ( !_err.empty())
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


// private
bool FaceDetector::findLandmarks( ObjMetaData::Ptr omd)
{
    const cv::Vec3f& ntip = omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);
    cv::Vec3f nvec, uvec;
    omd->getOrientation( nvec,uvec);

    RFeatures::CameraParams cp( 600*nvec + ntip, ntip, uvec);  // Set nose tip as focus
    _viewer->setCamera( cp);
    _viewer->updateRender();

    // Model aligned to maximise good detection of feature points
    const bool found = _impl->detectFeatures( _viewer, omd);
    if ( found)
        findBoundary( omd, 2.3);
    else
        _err = "Found only a subset of landmarks.";
    return found;
}   // end findLandmarks


