#include <CurvatureVariableSpeedFunctor.h>
using FaceTools::CurvatureVariableSpeedFunctor;
using RFeatures::ObjModelCurvatureMap;
using RFeatures::ObjModel;
#include <boost/foreach.hpp>
#include <cassert>

CurvatureVariableSpeedFunctor::CurvatureVariableSpeedFunctor( const ObjModelCurvatureMap::Ptr omcm)
{
    assert( omcm);
    const ObjModel::Ptr model = omcm->getObject();

    double kp1, kp2;
    const IntSet& vidxs = model->getVertexIds();
    BOOST_FOREACH ( const int& vi, vidxs)
    {
        omcm->getVertexPrincipalCurvature1( vi, kp1); // Magnitude of curvature in direction of max curvature
        omcm->getVertexPrincipalCurvature2( vi, kp2); // Magnitude of curvature in direction of minimum curvature
        _curvedness[vi] = sqrt( pow(kp1,2) + pow(kp2,2)/2);
    }   // end foreach
}   // end ctor
