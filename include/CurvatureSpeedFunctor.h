#ifndef FACE_TOOLS_CURVATURE_SPEED_FUNCTOR_H
#define FACE_TOOLS_CURVATURE_SPEED_FUNCTOR_H

#include "FaceTools_Export.h"
#include <ObjModelTools.h>

namespace FaceTools
{

class FaceTools_EXPORT CurvatureSpeedFunctor : public RFeatures::ObjModelFastMarcher::SpeedFunctor
{
public:
    CurvatureSpeedFunctor( const RFeatures::ObjModelCurvatureMap::Ptr omcm);

    virtual double operator()( int vidx) const { return _curvedness.at(vidx);}

private:
    boost::unordered_map<int, double> _curvedness;
};  // end class

}   // end namespace

#endif
