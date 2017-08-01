#include <CurvatureSpeedFunctor.h>
using FaceTools::CurvatureSpeedFunctor;
using RFeatures::ObjModelCurvatureMap;
using RFeatures::ObjModel;
#include <boost/foreach.hpp>
#include <cassert>


CurvatureSpeedFunctor::CurvatureSpeedFunctor( const ObjModelCurvatureMap::Ptr omcm)
{
    assert( omcm);
    const ObjModel::Ptr model = omcm->getObject();
    double meanCurvature = 0.0;

    double kp1, kp2, c;
    std::vector<double> vals;
    const IntSet& vidxs = model->getVertexIds();
    BOOST_FOREACH ( const int& vi, vidxs)
    {
        omcm->getVertexPrincipalCurvature1( vi, kp1); // Magnitude of curvature in direction of max curvature
        omcm->getVertexPrincipalCurvature2( vi, kp2); // Magnitude of curvature in direction of minimum curvature
        c = sqrt((pow(kp1,2) + pow(kp2,2))/2);
        _curvedness[vi] = c;
        vals.push_back(c);
        meanCurvature += c;
    }   // end foreach

    meanCurvature /= vidxs.size();
    const double stddev = RFeatures::calcStdDev( vals, meanCurvature, 0);
    const double threshold = meanCurvature + stddev;

    // Set the per vertex curvature function values
    BOOST_FOREACH ( const int& vi, vidxs)
        _curvedness[vi] = _curvedness[vi] > threshold ? 1.0 : 0.0;
}   // end ctor
