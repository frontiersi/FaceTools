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

#include <FacePointSampler.h>
#include <MiscFunctions.h>
#include <FaceTools.h>
#include <Landmarks.h>
#include <ObjModelVertexAdder.h>    // RFeatures
#include <DijkstraShortestPathFinder.h> // RFeatures
#include <sstream>
#include <cassert>
using FaceTools::FacePointSampler;
using FaceTools::ObjMetaData;
using RFeatures::ObjModel;


FacePointSampler::FacePointSampler( const ObjMetaData::Ptr omd) : _omd(omd)
{
}   // end ctor


int findBoundaryStartVertexIndex( const ObjMetaData::Ptr omd, const std::vector<int>& bvs)
{
    // Start at the top of the boundary (found as the boundary vertex that maximises the vector projection
    // with the vector rooted at the face centre and passing through the point midway between the eye centres).
    const ObjModel::Ptr model = omd->getObject();
    const cv::Vec3f eyeMid = 0.5f * (omd->getLandmark( FaceTools::Landmarks::L_EYE_CENTRE) + omd->getLandmark( FaceTools::Landmarks::R_EYE_CENTRE));
    const cv::Vec3f faceMid = FaceTools::calcFaceCentre( omd);
    cv::Vec3f upvec;    // NB don't use model orientation up vector here since it's calculation may change
    cv::normalize( eyeMid - faceMid, upvec);
    // Find the boundary starting point
    int bidx = 0;;  // Index into bv of best vertex to maximise local "up" projection
    double maxProj = 0.0;
    cv::Vec3f bv;   // Normalised boundary vector (rooted at face centre)
    double plen;
    const int n = (int)bvs.size();
    for ( int i = 0; i < n; ++i)
    {
        cv::normalize( model->vtx(bvs[i]) - faceMid, bv);
        plen = bv.dot( upvec);
        if ( plen >= maxProj)
        {
            maxProj = plen;
            bidx = i;
            // Go all the way around the boundary to get best. Could be smarter about this to stop the loop earlier, but
            // requires more convoluted logic since don't know where starting on the boundary. Besides, saving overall is minimal.
        }   // end if
    }   // end for
    return bidx;
}   // end findBoundaryStartVertexIndex


void findRingRefPoints( ObjMetaData::Ptr omd, const std::vector<int>& dends, int H, std::vector< std::vector<int> >& cRingRefPoints)
{
    cRingRefPoints.resize(H-1);

    const ObjModel::Ptr model = omd->getObject();
    RFeatures::DijkstraShortestPathFinder dspf( model);

    const int nvidx = omd->getKDTree()->find( omd->getLandmark( FaceTools::Landmarks::NASAL_TIP));    // Vertex closest to nose tip
    const int n = (int)dends.size();
    int sp;
    std::vector<int> spvidxs;       // Shortest path vertices
    std::vector<int> gSpokePoints;  // H equidistant points from spvidxs
    for ( int i = 0; i < n; ++i)    // For the endpoints of each spoke
    {
        dspf.setEndPointVertexIndices( nvidx, dends[i]);    // From boundary to nose tip
        dspf.findShortestPath( spvidxs);
        FaceTools::getEquidistant( model, spvidxs, 0, H, gSpokePoints);  // Points at endpoints of path set too
        // Set the consecutive reference points on each ring where position along spoke (j) == ring index.
        for ( int j = 0; j < H-2; ++j)
        {
            sp = gSpokePoints[j];
            // Don't duplicate reference points on a ring!
            if ( cRingRefPoints[j].empty() || cRingRefPoints[j].back() != sp)
                cRingRefPoints[j].push_back( sp);
        }   // end for
    }   // end for

    // DEBUG
    for ( int j = 0; j < H-2; ++j)
    {
        const int n = (int)cRingRefPoints[j].size();
        std::cerr << "Ring " << j << " has " << n << " points" << std::endl;
        for ( int k = 0; k < n; ++k)
        {
            std::ostringstream oss;
            oss << j << "_RING " << " " << k;
            omd->setLandmark( oss.str(), model->vtx( cRingRefPoints[j][k]));
        }   // end for
    }   // end for
}   // end findRingRefPoints


// Find the N sampled points on the mesh. Returns the actual number of points sampled
// which will be N approx D*(H+1)/2 where D and H are the adjacent integers nearest
// to giving N. Returns <= 0 if points can't be extracted for any reason. NB the points
// that are extracted are the original vertices nearest to the sampling locations
// so for accuracy, the given model should be a dense mesh.
int FacePointSampler::extractPoints( int N)
{
    const ObjModel::Ptr model = _omd->getObject();
    // Find D points equally spaced on the cropped boundary of the model.
    RFeatures::ObjModelTriangleMeshParser parser( model);
    RFeatures::ObjModelBoundaryFinder bfinder;
    parser.setBoundaryParser( &bfinder);
    parser.parse();
    assert( bfinder.getNumBoundaries() == 1);   // after processing, there should only be a single boundary around the face
    const std::list<int>& bverts = bfinder.getBoundary(0);
    std::vector<int> bvs( bverts.begin(), bverts.end());

    const int H = (int)sqrt(2*N); // Number of rings (rings numbered from 1 to H going from inner to outer rings).
    const int D = H+1;            // Number of points on the outer ring (will be scaled as i/H with ring number i).

    // Get the ends of the radial spokes starting at the top of the face boundary
    const int j = findBoundaryStartVertexIndex( _omd, bvs);
    std::vector<int> spokeEnds; // Radial spoke ends (4 spokes)
    FaceTools::getEquidistant( model, bvs, j, 8, spokeEnds);

    // Get the consecutive positions on each of the concentric rings that will be used as endpoints
    // for shortest path calculations. Find H+1 rings because we don't care about the endpoint
    // (nosetip) and only want the first H rings
    std::vector<std::vector<int> > cRingRefPoints;
    findRingRefPoints( _omd, spokeEnds, H+1, cRingRefPoints);

    int tpoints = 0;

    /*
    // For each entry in cRingRefPoints, create bounding rings by connecting consecutive points using A* Dijkstra's
    _crings.resize(H);
    RFeatures::DijkstraShortestPathFinder dspf( model);
    for ( int h = 0; h < H; ++h)    // For each ring
    {
        // For each consecutive pair of points (i,j) along ring h, get the shortest path on the face.
        const std::vector<int>& refPoints = cRingRefPoints[h];  // Ref points along ring h
        const int n = (int)refPoints.size();
        std::vector<int> cring;  // Will be the complete path
        for ( int i = 0, j = 1; j < n; ++i, ++j)
        {
            dspf.setEndPointVertexIndices( refPoints[j], refPoints[i]);
            dspf.findShortestPath( cring, false);   // Don't clear before adding
        }   // end for

        // For each ring, find the D*(H-h)/H equidistant points along it
        const int npoints = D*double(H-h)/H;    // Floor
        FaceTools::getEquidistant( model, cring, 0, npoints, _crings[h]);
        assert( int(_crings[h].size()) == npoints);
        tpoints += npoints;
    }   // end foreach
    */

    return tpoints;
}   // end extractPoints

