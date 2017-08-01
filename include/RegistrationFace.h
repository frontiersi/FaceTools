#ifndef FACE_TOOLS_REGISTRATION_FACE_H
#define FACE_TOOLS_REGISTRATION_FACE_H

#include "FaceTools_Export.h"
#include <ObjModelCurvatureMap.h>         // RFeatures
#include <ObjModelFaceAngleCalculator.h>  // RFeatures
#include <ObjModelFastMarcher.h>          // RFeatures

namespace FaceTools
{

class FaceTools_EXPORT RegistrationFace
{
public:
    // Provided curvature map must be for a pre-processed face model
    // (i.e., triangulated mesh, no holes, dense vertices, as smooth as required).
    explicit RegistrationFace( const RFeatures::ObjModelCurvatureMap::Ptr);

    // Sample n thresholded high curvature points from the face.
    RFeatures::ObjModel::Ptr sampleHighCurvature( int n);

    // Uniformly sample n points where points are not within dthresh units of any points in f1.
    RFeatures::ObjModel::Ptr sampleUniformly( int n);

    // Does interpolated sampling to get n interpolated vertices (no polygons added).
    RFeatures::ObjModel::Ptr sampleUniformlyInterpolated( int n);

    // Use the variable curvature speed functor to sample higher curvature areas more densely.
    RFeatures::ObjModel::Ptr sampleCurvatureVariableInterpolated( int n);

    // Return edges between sample points for the last call to one of the sampling functions.
    const boost::unordered_map<int,IntSet>& getEdges() const { return _sedges;}

private:
    const RFeatures::ObjModelCurvatureMap::Ptr _curvmap;    // References original high vertex density pre-processed face
    RFeatures::FaceAngles _faceAngles;
    boost::unordered_map<int,IntSet> _sedges;

    RFeatures::ObjModel::Ptr sample( int, RFeatures::ObjModelFastMarcher::SpeedFunctor*);
    RFeatures::ObjModel::Ptr sampleInterp( int, RFeatures::ObjModelFastMarcher::SpeedFunctor*);

    RegistrationFace( const RegistrationFace&); // NO COPY
    void operator=( const RegistrationFace&);   // NO COPY
};  // end class

}   // end namespace

#endif
