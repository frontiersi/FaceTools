/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <FacialOrientation.h>
#include <DijkstraShortestPathFinder.h>
#include <ObjModelNormals.h>
using RFeatures::ObjModelKDTree;
using RFeatures::ObjModel;
#include <algorithm>


namespace {

cv::Vec3d calcMeanNormalBetweenPoints( const ObjModel* model, int v0, int v1)
{
    RFeatures::DijkstraShortestPathFinder dspf( model);
    dspf.setEndPointVertexIndices( v0, v1);
    std::vector<int> vidxs;
    dspf.findShortestPath( vidxs);
    const int n = (int)vidxs.size() - 1;
    cv::Vec3d nrm(0,0,0);
    for ( int i = 0; i < n; ++i)
    {
        const IntSet& sfids = model->getSharedFaces( vidxs[i], vidxs[i+1]);
        std::for_each( std::begin(sfids), std::end(sfids), [&](int fid){
            nrm += RFeatures::ObjModelNormals::calcNormal( model, fid);});
    }   // end for
    cv::normalize( nrm, nrm);
    return nrm;
}   // end calcMeanNormalBetweenPoints


cv::Vec3d findNormalEstimate( const ObjModelKDTree::Ptr kdt, const cv::Vec3d& inNorm, int e0, int e1)
{
    const ObjModel* model = kdt->model();
    const cv::Vec3d v0 = model->vtx(e0);
    const cv::Vec3d v1 = model->vtx(e1);

    // Estimate "down" vector from cross product of base vector with current (inaccurate) face normal.
    const cv::Vec3d baseVec = v1 - v0;
    cv::Vec3d estDownVec;
    cv::normalize( baseVec.cross(inNorm), estDownVec);

    // Find locations further down the face from e0 and e1
    const double pdelta = 1.0 * cv::norm(baseVec);
    const int c0 = kdt->find( v0 + estDownVec * pdelta);
    const int c1 = kdt->find( v1 + estDownVec * pdelta);

    // The final view vector is defined as the mean normal along the path over
    // the model between the provided points and the shifted points.
    const cv::Vec3d vv0 = calcMeanNormalBetweenPoints( model, c0, e0);
    const cv::Vec3d vv1 = calcMeanNormalBetweenPoints( model, c1, e1);
    cv::Vec3d viewNorm;
    cv::normalize( vv0 + vv1, viewNorm);
    return viewNorm;
}   // end findNormalEstimate

}   // end namespace


// public
bool FaceTools::Detect::findOrientation( const ObjModelKDTree::Ptr kdt,
                                         const cv::Vec3f& v0, const cv::Vec3f& v1,
                                         cv::Vec3f& normvec, cv::Vec3f& upvec)
{
    const ObjModel* model = kdt->model();
    const int e0 = kdt->find( v0);
    const int e1 = kdt->find( v1);

    cv::Vec3d viewNorm = cv::Vec3d(0,0,1);  // Initial view normal estimate as +Z
    cv::Vec3d inNorm = viewNorm;
    const double MIN_DELTA = 1e-8;
    double delta = MIN_DELTA + 1;
    const int MAX_TRIES = 12;
    int tries = 0;
    while ( fabs(delta) > MIN_DELTA && tries < MAX_TRIES)
    {
        inNorm = viewNorm;
        const cv::Vec3d nextNorm = findNormalEstimate( kdt, inNorm, e0, e1);
        cv::normalize( inNorm + nextNorm, viewNorm); // View norm for next iteration is average of input and output
        delta = cv::norm( viewNorm - inNorm);
        tries++;
    }   // end while

    normvec[0] = (float)viewNorm[0];
    normvec[1] = (float)viewNorm[1];
    normvec[2] = (float)viewNorm[2];
    cv::Vec3d nuvec;
    cv::normalize( viewNorm.cross( v1 - v0), nuvec);  // Set the normalised up vector
    upvec[0] = (float)nuvec[0];
    upvec[1] = (float)nuvec[1];
    upvec[2] = (float)nuvec[2];

    return true;
}   // end findOrientation


// public
cv::Vec3d FaceTools::Detect::calcUp( const cv::Vec3f& nbridge, const cv::Vec3f& eye0, const cv::Vec3f& eye1)
{
    const cv::Vec3d lv = eye0 - nbridge;
    const cv::Vec3d rv = eye1 - nbridge;
    cv::Vec3d up;
    cv::normalize( rv.cross(lv), up);
    return up;
}   // end calcUp


// public
cv::Vec3d FaceTools::Detect::calcNormal( const cv::Vec3f& upv, const cv::Vec3f& eye0, const cv::Vec3f& eye1)
{
    cv::Vec3d norm;
    cv::normalize( upv.cross( eye0 - eye1), norm);
    return norm;
}   // end calcNormal
