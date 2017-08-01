#include <FaceRegistrationMapper.h>
using FaceTools::FaceRegistrationMapper;
using FaceTools::ObjMetaData;
using RFeatures::ObjModel;
#include <FaceTools.h>


FaceRegistrationMapper::FaceRegistrationMapper( ObjMetaData::Ptr omd) : _omd(omd) {}


void FaceRegistrationMapper::generate()
{
    using namespace RFeatures;
    // Crop around point between the eyes (in XY plane of eyes) and immediately behind nose tip.
    ObjModel::Ptr cmodel = FaceTools::cropAroundFaceCentre( _omd, 2.2);
    // Increase vertex density to max of 2mm separation
    ObjModelVertexAdder::addVertices( cmodel, 2.0);

    // Super-smooth
    ObjModelCurvatureMap::Ptr cmap = ObjModelCurvatureMap::create( cmodel, 0);
    size_t nSmoothOps = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( 0.001, nSmoothOps);
}   // end generate
