#include <FaceTools.h>
#include <FaceModelManager.h>
#include <FaceModelOBJFileHandler.h>
#include <FaceModelXMLFileHandler.h>
#include <FaceOrientationDetector.h>
#include <LandmarksManager.h>
using FaceTools::FM;

static const std::string BASE_DIR = "/home/rich/lb/Cliniface/Release/bin/data";
static const std::string lmksFile = BASE_DIR + "/lmks.txt";
static const std::string haarModels = BASE_DIR + "/facemodels/haarcascades";
static const std::string faceShapeL = BASE_DIR + "/facemodels/shape_predictor_68_face_landmarks.dat";


FM* load( const char* fname)
{
    using namespace FaceTools::FileIO;
    FMM::add( new FaceModelXMLFileHandler);
    FMM::add( new FaceModelOBJFileHandler);
    FM* fm = FMM::read( fname);
    if ( !fm)
        std::cerr << "Unable to read in FaceModel from " << fname << std::endl;
    return fm;
}   // end load


int main( int argc, char *argv[])
{
    if ( argc == 1)
    {
        std::cerr << "Pass in model filename" << std::endl;
        return EXIT_FAILURE;
    }   // end if

    FM* fm = load( argv[1]);
    if ( !fm)
        return EXIT_FAILURE;

    FaceTools::Landmark::LandmarksManager::load( lmksFile);
    using namespace FaceTools::Detect;

    FeaturesDetector::initialise( haarModels);
    FaceShapeLandmarks2DDetector::initialise( faceShapeL);

    FaceOrientationDetector fdetector( fm->kdtree(), 650.0f, 0.3f);
    fdetector.detect( *fm->landmarks());

    return EXIT_SUCCESS;
}   // end main
