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

#include <FaceOrienter.h>
#include <DijkstraShortestPathFinder.h>
using FaceTools::FaceOrienter;
using RFeatures::ObjModelCurvatureMap;
using RFeatures::ObjModelKDTree;
using RFeatures::ObjModel;
#include <cassert>


// public static
cv::Vec3d FaceOrienter::calcUp( const cv::Vec3f& nbridge, const cv::Vec3f& eye0, const cv::Vec3f& eye1)
{
    const cv::Vec3d lv = eye0 - nbridge;
    const cv::Vec3d rv = eye1 - nbridge;
    cv::Vec3d up;
    cv::normalize( rv.cross(lv), up);
    return up;
}   // end calcUp


// public static
cv::Vec3d FaceOrienter::calcNormal( const cv::Vec3f& upv, const cv::Vec3f& eye0, const cv::Vec3f& eye1)
{
    cv::Vec3d norm;
    cv::normalize( upv.cross( eye0 - eye1), norm);
    return norm;
}   // end calcNormal


cv::Vec3d FaceOrienter::calcMeanNormalBetweenPoints( int v0, int v1) const
{
    const ObjModel::Ptr model = _curvMap->getObject();
    RFeatures::DijkstraShortestPathFinder dspf( model);
    dspf.setEndPointVertexIndices( v0, v1);
    std::vector<int> vidxs;
    dspf.findShortestPath( vidxs);
    const int n = (int)vidxs.size();
    cv::Vec3d nrm;
    for ( int i = 0; i < n; ++i)
        nrm += _curvMap->getVertexNormal( vidxs[i]);
    cv::normalize( nrm, nrm);
    return nrm;
}   // end calcMeanNormalBetweenPoints


cv::Vec3d FaceOrienter::findNormalEstimate( const cv::Vec3d& inNorm, int e0, int e1) const
{
    const ObjModel::Ptr model = _curvMap->getObject();
    const cv::Vec3d v0 = model->getVertex(e0);
    const cv::Vec3d v1 = model->getVertex(e1);

    // Estimate the "down" vector from the cross product of the base vector
    // with the current (inaccurate) face normal.
    const cv::Vec3d baseVec = v1 - v0;
    cv::Vec3d estDownVec;
    cv::normalize( baseVec.cross(inNorm), estDownVec);

    // Find locations further down the face from e0 and e1
    const double pdelta = 1.0 * cv::norm(baseVec);
    const int c0 = _kdtree->find( v0 + estDownVec * pdelta);
    const int c1 = _kdtree->find( v1 + estDownVec * pdelta);

    // The final view vector is defined as the mean normal along the path over
    // the model between the provided points and the shifted points.
    const cv::Vec3d vv0 = calcMeanNormalBetweenPoints( c0, e0);
    const cv::Vec3d vv1 = calcMeanNormalBetweenPoints( c1, e1);
    //const cv::Vec3d vv2 = calcMeanNormalBetweenPoints( c0, e1);
    //const cv::Vec3d vv3 = calcMeanNormalBetweenPoints( c1, e0);
    cv::Vec3d viewNorm;
    //cv::normalize( vv0 + vv1 + vv2 + vv3, viewNorm);
    cv::normalize( vv0 + vv1, viewNorm);
    return viewNorm;
}   // end findNormalEstimate


// public
bool FaceOrienter::operator()( const cv::Vec3f& v0, const cv::Vec3f& v1, cv::Vec3d& normvec, cv::Vec3d& upvec) const
{
    const ObjModel::Ptr model = _curvMap->getObject();
    const int e0 = _kdtree->find( v0);
    const int e1 = _kdtree->find( v1);

    // Get the initial normal based upon an average of the vertex normals between the pupils.
    // This normal typically points down more than straight out from the face, but it is good
    // enough to find other points on the model that can be used to refine the normal.
    cv::Vec3d viewNorm = cv::Vec3d(0,0,1);
    cv::Vec3d inNorm = viewNorm;
    const double MIN_DELTA = 1e-8;
    double delta = MIN_DELTA + 1;
    const int MAX_TRIES = 12;
    int tries = 0;
    while ( fabs(delta) > MIN_DELTA && tries < MAX_TRIES)
    {
        inNorm = viewNorm;
        const cv::Vec3d nextNorm = findNormalEstimate( inNorm, e0, e1);
        // View norm for next iteration is average of the input and the output
        cv::normalize( inNorm + nextNorm, viewNorm);
        delta = cv::norm( viewNorm - inNorm);
        //std::cerr << tries << ") delta = " << delta
        //          << " :  in" << inNorm << " ---> out" << viewNorm << std::endl;
        tries++;
    }   // end while

    normvec = viewNorm;
    cv::normalize( viewNorm.cross( v1 - v0), upvec);  // Set the normalised up vector
    return true;
}   // end operator()


// public
FaceOrienter::FaceOrienter( const ObjModelCurvatureMap::Ptr cm, const ObjModelKDTree::Ptr kdt)
    : _curvMap(cm), _kdtree( kdt != NULL ? kdt : ObjModelKDTree::create(cm->getObject()))
{
}   // end ctor

