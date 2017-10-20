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

#include <NoseFinder.h>
#include <FaceOrienter.h>
#include <MiscFunctions.h>
using FaceTools::NoseFinder;
#include <cassert>
#include <numeric>
#include <boost/foreach.hpp>
#include <DijkstraShortestPathFinder.h> // RFeatures
using RFeatures::ObjModel;

/*
#include <SobelMaker.h>

cv::Mat_<float> makeSobelMap( const cv::Mat_<float>& zbuff, const cv::Rect& noseBox)
{
    // Convert the depth buffer to byte
    cv::Mat_<float> nbuff = zbuff(noseBox).clone();
    double mn, mx;
    cv::minMaxLoc( nbuff, &mn, &mx);
    nbuff -= mn;
    cv::Mat_<byte> bbuff(nbuff.size());
    nbuff.convertTo( bbuff, CV_8U, 255./(mx-mn));
    // Median blur to get rid of possible pixel holes
    cv::medianBlur(bbuff, bbuff, 3);
    // Get the first derivative map
    RFeatures::SobelMaker sm( bbuff);
    cv::Mat_<float> rawzd = sm.makeSobelD1( 3); // ksize = 3

    // Find the brightest points in the left and right halves of rawzd and combine
    const cv::Rect lrect( 0, 0, rawzd.cols/2, rawzd.rows);
    const cv::Rect rrect( lrect.width, 0, rawzd.cols-lrect.width, rawzd.rows);
    cv::minMaxLoc( rawzd(lrect), &mn, &mx);
    rawzd(lrect) -= mn;
    rawzd(lrect) /= (mx-mn);
    cv::minMaxLoc( rawzd(rrect), &mn, &mx);
    rawzd(rrect) -= mn;
    rawzd(rrect) /= (mx-mn);

    return rawzd;
}   //  end makeSobelMap


cv::Mat_<byte> makeBinaryEdgeMap( const cv::Mat_<float>& rawzd)
{
    double mn, mx;
    cv::minMaxLoc( rawzd, &mn, &mx);
    return rawzd > mx/3;
}   // end makeBinaryEdgeMap
*/


// public
NoseFinder::NoseFinder( const RFeatures::ObjModelCurvatureMap::Ptr cm, int e0, int e1)
    : _curvMap(cm), _model(cm->getObject()), _e0(e0), _e1(e1), _midEyes(0,0,0), _nbridge(0,0,0), _ntip(0,0,0)
{}   // end ctor


// private
bool NoseFinder::calcMetrics( int uvidx, const cv::Vec3f& origVtx, bool growingNR,
                              double& bestNRHeight, double& bestDelta) const
{
    const cv::Vec3f& v = _model->getVertex(uvidx);
    const cv::Vec3f& ve0 = _model->getVertex( _e0);
    const cv::Vec3f& ve1 = _model->getVertex( _e1);
    const double n0 = cv::norm(v - ve0);
    const double n1 = cv::norm(v - ve1);
    const double eyeBaseLen = cv::norm( ve1 - ve0);
    // Vertex height from eye base vector for when growing nose ridge
    const double nrheight = 2 * RFeatures::calcTriangleArea( n0, n1, eyeBaseLen) / eyeBaseLen;
    // Distance from start point minus distance difference between eyes.
    const double delta = cv::norm(v - origVtx) - pow(n0 - n1,2);

    bool betterDeltas = false;
    if (( growingNR && nrheight >= bestNRHeight) || ( !growingNR && delta >= bestDelta))
    {
        bestDelta = delta;
        bestNRHeight = nrheight;
        betterDeltas = true;
    }   // end if
    return betterDeltas;
}   // end calcMetrics


// private
int NoseFinder::searchForNoseTip( int origUvidx, cv::Vec3d& dvec, double degsAllowed)
{
    int ntip = -1;
    _noseRidgePath.clear();

    // Iteratively find the connected vertex to gc that is within the allowed
    // direction cone and that maximises the distances from e0 and e1 while
    // minimising the difference in distance between those points. Iteration
    // stops when the distance grown from the eyes is less than a minimum proportion
    // of the edge distance from the previous to the next vertex.
    double MAX_RADS_ALLOWED = degsAllowed * CV_PI/180;
    double MIN_GROW_GAIN = 0.4;

    // Don't reparse vertices already on the path
    boost::unordered_set<int> pathVerts;
    bool growingNR = false;   // Set true when nose tip grow exceeds min average growth rate

    // Growth rate is defined using the preceeding three vertices.
    // While growingNR is false (i.e. the nose ridge height from the eye vector
    // base line is too small), the metric grown is the straight line distance from
    // the starting point while minimising the difference in distances to the eyes.

    // Location of the starting vertex and original metrics
    cv::Vec3f origVtx = _model->getVertex(origUvidx);
    double bestDelta = -DBL_MAX;
    double bestNRHeight = -DBL_MAX;
    calcMetrics( origUvidx, origVtx, growingNR, bestNRHeight, bestDelta);

    // Keep track of the most recent three vertices so can always refer to the vertex from
    // three iterations previous in order to calculate straight line distance for the purpose
    // of measuring growth rate to the nose tip.
    std::vector<cv::Vec3f> lastVtxs(3, origVtx);
    std::vector<double> lastNRs(3, bestNRHeight);
    int tidx = 0;

    int nextUvidx = origUvidx;
    int currUvidx;
    bool foundNoseTip = false;
    while ( true)
    {
        currUvidx = nextUvidx;
        _noseRidgePath.push_back(currUvidx);
        pathVerts.insert(currUvidx);   // Don't process this vertex again since it's now on the path
        const IntSet& conns = _model->getConnectedVertices(currUvidx);

        int nvalid = 0;
        double minRads = DBL_MAX;
        bestDelta = -DBL_MAX;
        bestNRHeight = -DBL_MAX;
        BOOST_FOREACH ( const int& connUvidx, conns)
        {
            if ( pathVerts.count(connUvidx))   // Ignore this vertex if already on path
                continue;

            // Ignore vertices not within search cone defined using current vertex
            const cv::Vec3d vdiff = _model->getVertex(connUvidx) - origVtx;  // Delta vec from start
            const double rads = acos( vdiff.dot(dvec) / cv::norm(vdiff));

            if ( rads <= MAX_RADS_ALLOWED)
            {
                if ( calcMetrics( connUvidx, origVtx, growingNR, bestNRHeight, bestDelta))
                    nextUvidx = connUvidx;
                nvalid++;
            }   // end if

            if ( nvalid == 0 && rads <= minRads)
            {
                minRads = rads;
                nextUvidx = connUvidx;
            }   // end if
        }   // end foreach

        if ( nextUvidx == currUvidx)    // Failed to find the nose tip!
            break;

        // If none of the connected vertices are within the search cone,
        // choose the closest to the centre line of the search cone.
        if ( nvalid == 0)
        {
            assert( bestNRHeight == -DBL_MAX);
            assert( bestDelta == -DBL_MAX);
            calcMetrics( nextUvidx, origVtx, growingNR, bestNRHeight, bestDelta);
        }   // end if

        // Get the nose ridge height growth rate taken from this vertex and the vertex three previous
        const cv::Vec3f& nextVtx = _model->getVertex(nextUvidx);
        const double nrGain = bestNRHeight - lastNRs[tidx];
        const double gmetric = nrGain / cv::norm(nextVtx - lastVtxs[tidx]);

        if ( !growingNR && gmetric >= MIN_GROW_GAIN)
        {
            growingNR = true;
            // Start point reset to point at which required growth first realised.
            origVtx = lastVtxs[tidx];
            // Allow greater flexibility in choosing the next vertex within
            // the search "cone" - greater than 180 degrees - to allow the path
            // to the nose tip to move back on itself.
            MAX_RADS_ALLOWED = CV_PI;
            _nbridge = _model->getVertex(nextUvidx);
        }   // end if

        assert( !cvIsNaN( gmetric));

        if ( growingNR && gmetric < MIN_GROW_GAIN)
        {
            ntip = nextUvidx;
            break;
        }   // end if

        lastVtxs[tidx] = nextVtx;
        lastNRs[tidx] = bestNRHeight;
        tidx = (tidx + 1) % lastVtxs.size();

        // Adjust the direction of the vector giving the search cone's height to be more
        // aligned with the nose ridge itself from the point at which significant growth
        // in the nose ridge height starts, to the current best growth vertex. The fixed
        // radius of the cone ensures that as the distance from the initial point vg increases,
        // the greater the flexibility the algorithm has for choosing vertices away from the
        // centre line of the cone (i.e. easier to adjust to find the true nose ridge / tip).
        if ( growingNR)
            cv::normalize( cv::Vec3d(nextVtx - origVtx), dvec);
    }   // end while

    return ntip;
}   // end searchForNoseTip


// public
bool NoseFinder::find()
{
    if ( _e0 == _e1)
        return false;

    RFeatures::DijkstraShortestPathFinder dspf( _model);
    if ( !dspf.setEndPointVertexIndices( _e1, _e0))
        return false;

    std::vector<int> spidxs;
    if ( dspf.findShortestPath(spidxs) <= 0)
        return false;

    const cv::Vec3f& ve0 = _model->getVertex(_e0);
    const cv::Vec3f& ve1 = _model->getVertex(_e1);

    // Get the start point as the point midway between the eye centres along the shortest path.
    // Find the down vector as the cross product of the normal at this point and the vector
    // positions of the eyes. The down vector doesn't have to be a great estimate since it's
    // only needed to help define a search cone with relatively wide radius.
    const int midEyesVidx = FaceTools::findMidway( _model, spidxs);
    _midEyes = _model->getVertex( midEyesVidx);
    const cv::Vec3d& onorm = _curvMap->getVertexNormal( midEyesVidx);
    const cv::Vec3d leftVec = ve0 - _midEyes;  // Points left from midpoint
    const cv::Vec3d rightVec = ve1 - _midEyes; // Points right from midpoint
    cv::Vec3d dvec; // Calculate down vector estimate
    cv::normalize( onorm.cross( leftVec) + rightVec.cross(onorm), dvec);

    // Allow for possible vertices to be grown down from the centre point
    // within a 90 degree cone centred on dvec. dvec must have unit length.
    int ntip = searchForNoseTip( midEyesVidx, dvec, 45);
    if ( ntip < 0)
        return false;

    // The nose tip is still not quite found since the above algorithm stops
    // once the growth rate of the "nose tip" from the eye base vector falls
    // under a fixed value. This means that the "nose tip" in its current
    // placement is probably around the nostrils or to one side of the nose
    // tip. Assuming the nose tip itself is concave, we can find a
    // vector that points out from the face given by the cross product of the
    // difference vector between the existing found nose bridge and nose tip
    // points, and the eye vector. This vector will point in the direction of
    // growth we want the final nose tip to be in. We iteratively find vertices
    // that grow in this direction until no closer vertex can be found.
    const cv::Vec3f upv = FaceTools::FaceOrienter::calcUp( _nbridge, ve0, ve1);
    cv::Vec3f growVec0 = FaceTools::FaceOrienter::calcNormal( upv, ve0, ve1);
    cv::Vec3f growVec;
    float NR_WEIGHT = 0.0005f; // How much of the normal defined by the nose ridge to use
    const cv::Vec3f& ntv = _model->getVertex( ntip);
    cv::normalize( NR_WEIGHT*(( ntv - _model->getVertex(_noseRidgePath[0])).cross( ve1 - ve0)) + (1.0f - NR_WEIGHT)*growVec0, growVec);

    _ntip = _model->getVertex( FaceTools::growOut( _model, growVec, ntip));
    return true;
}   // end find


/*
cv::Point findMaxNostrilPlacement( const cv::Point& sp, const cv::Point& noseTip, const cv::Mat_<byte>& nmask)
{
    const cv::Rect maskRct( 0, 0, nmask.cols, nmask.rows);
    // sp.x is decremented if sp is already to the left of the noseTip (left nostril)
    // and sp.x is incremented if sp is already to the right of the noseTip (right nostril)
    const int xinc = sp.x < noseTip.x ? -1 : 1;

    cv::Point tp = sp;  // Test point
    double maxDist = cv::norm( tp - noseTip);   // To be maximised

    cv::Point oldp = sp;
    cv::Point np = oldp;
    double curDist = maxDist;

    while ( maskRct.contains(np) && curDist >= maxDist)
    {
        maxDist = curDist;
        oldp = np;
        np.y++; // Go down a pixel
        if ( !nmask.at<byte>(np))   // Ensure still in the mask area (white region)
            np.x += xinc;
        if ( !nmask.at<byte>(np))   // If moving one pixel further out did not keep us on the mask area, we're done
            break;

        // Ensure np is right at the edge of the mask region furthest from the nose tip
        while ( maskRct.contains(np) && nmask.at<byte>(np))
            np.x += xinc;
        np.x -= xinc;

        tp = np;    // New test point
        curDist = cv::norm( tp - noseTip);
    }   // end while

    return oldp;
}   // end findMaxNostrilPlacement


bool findStartingPositionsForNostrils( const cv::Mat_<byte>& nmask, cv::Point& lp, cv::Point& rp)
{
    // Find starting horizontal line in nmask
    lp = rp = cv::Point(0,0);
    int i = nmask.rows/2;
    while ( i < nmask.rows && (lp.y == 0 || rp.y == 0))
    {
        const byte* mrow = nmask.ptr<byte>(i);

        if ( lp.y == 0)
        {
            for ( int j = 0; j < nmask.cols/2; ++j)
            {
                if ( mrow[j])
                {
                    lp = cv::Point(j,i);
                    break;
                }   // end if
            }   // end for
        }   // end if

        if ( rp.y == 0)
        {
            for ( int j = nmask.cols-1; j > nmask.cols/2; --j)
            {
                if ( mrow[j])
                {
                    rp = cv::Point(j,i);
                    break;
                }   // end if
            }   // end for
        }   // end if

        i++;
    }   // end while

    // If no nostrils marks found
    if ( i == nmask.rows)
        return false;

    return true;
}   // end findStartingPositionsForNostrils


// public
bool NoseFinder::findNostrils( const cv::Rect_<float>& sarea)
{
    const cv::Mat_<float> zbuff = _zbuff;
    const cv::Size msz = zbuff.size();

    const cv::Point noseTip = FaceApp::fromProportion( _ntip, msz);
    assert( noseTip.x > 0 || noseTip.y > 0);
    if ( noseTip.x == 0 && noseTip.y == 0)
        return false;

    const cv::Rect searchArea = FaceApp::fromProportion( sarea, msz);

    if ( !RFeatures::isWithin(zbuff,searchArea))
    {
        std::cerr << "ERROR: FaceApp::NoseFinder::findNostrils: given searchArea not within image bounds!" << std::endl;
        return false;
    }   // end if

    const cv::Point relNoseTip( noseTip.x - searchArea.x, noseTip.y - searchArea.y);

    const cv::Mat_<float> rawzd = makeSobelMap( zbuff, searchArea);
    //RFeatures::showImage( RFeatures::contrastStretch(rawzd), "NoseFinder::findNostrils", false);

    const cv::Mat_<byte> nmask = makeBinaryEdgeMap( rawzd);  // Nose mask covering searchArea only
    cv::Point lp, rp;
    if ( !findStartingPositionsForNostrils( nmask, lp, rp))
    {
        std::cerr << "WARNING: FaceApp::NoseFinder::findNostrils: unable to find starting positions for left and right nostril search!" << std::endl;
        return false;
    }   // end if

    // Now follow each point down and around the contour of the nostril while the distance between
    // the nostril point and the nose tip is being maximised in 3D
    cv::Point ln = findMaxNostrilPlacement( lp, relNoseTip, nmask);
    cv::Point rn = findMaxNostrilPlacement( rp, relNoseTip, nmask);
    ln.x += searchArea.x;
    ln.y += searchArea.y;
    rn.x += searchArea.x;
    rn.y += searchArea.y;
    _lnostril = FaceApp::toProportion( ln, msz);
    _rnostril = FaceApp::toProportion( rn, msz);
    return true;
}   // end findNostrils
*/
