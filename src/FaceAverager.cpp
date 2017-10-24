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

#include <FaceAverager.h>
#include <FaceTools.h>
#include <Landmarks.h>
#include <ObjModelEdgeFaceAdder.h>  // RFeatures
#include <ObjModelRemesher.h>       // RFeatures
#include <ObjModelHoleFiller.h>     // RFeatures
#include <CpuTimer.h>       // rlib
#include <cpd/nonrigid.hpp>    // Coherent Point Drift
using FaceTools::FaceAverager;
using RFeatures::ObjModel;
#include <iostream>
#include <boost/foreach.hpp>


FaceAverager::Ptr FaceAverager::create( int n) { return Ptr( new FaceAverager(n));}


FaceAverager::FaceAverager( int n)
{
    _cmat = cv::Mat_<cv::Vec3f>(0, n);  // n correspondence points (columns)
}   // end ctor

FaceAverager::~FaceAverager() {}


// public
int FaceAverager::add( const ObjModel::Ptr model)
{
    int nrow = 0;
    if ( _cmat.rows == 0)
        init( model);
    else
    {
        nrow = _cmat.rows;
        _cmat.resize( nrow + 1);   // Create a new row (may be first)
        // Perform non-rigid transform
    }   // end else

    return nrow;
}   // end add

/*
int FaceAverager::addLandmarks( const ObjMetaData::Ptr omd)
{
    boost::unordered_set<std::string> lnames;
    const int n = (int)omd->getLandmarks( lnames);
    if ( n > 0)
    {
        int lvidx, bfid;
        cv::Vec3f sv;   // Projected surface vertex (not used)
        RFeatures::ObjModelSurfacePointFinder sfinder( _model);
        RFeatures::ObjModelKDTree::Ptr kdtree = RFeatures::ObjModelKDTree::create(_model);
        BOOST_FOREACH ( const std::string& lname, lnames)
        {
            const cv::Vec3f& lv = omd->getLandmark(lname);
            lvidx = kdtree->find(lv); // Find the nearest vertex on the sample model
            sfinder.find( lv, lvidx, bfid, sv); // Project into model surface starting at lvidx
            // If bfid -1, landmark is at existing vertex so no need to subdivide,
            // otherwise subdivide the identified triangle which created a new vertex.
            if ( bfid >= 0)
                lvidx = _model->subDivideFace( bfid, lv);

            _lmks[lname] = lvidx;
        }   // end foreach
    }   // end if
    return _model->getNumVertices();
}   // end addLandmarks
*/


void FaceAverager::init( const ObjModel::Ptr model)
{
    using namespace RFeatures;

    ObjModelRemesher *remesher;
    { rlib::CpuTimer cputimer( "[INFO] FaceTools::FaceAverager::init: Model sampling", std::cerr);
        ObjModelFastMarcher::SpeedFunctor constantFunctor;    // Unweighted speed weightings for distance mapping
        remesher = new ObjModelRemesher( model, &constantFunctor);
        remesher->sample( 0, _cmat.cols);
        _model = remesher->getSampleObject();
    }   // end timed section

    // NB FaceTools::SurfaceMesher takes too long and often sets a band at the back of the face.
    // Using the saddle edges from ObjModelRemesher is also buggy right not (can result in a manifold with holes).
    // The saddle edges also cannot be used with a model produced via interpolated sampling. However, this
    // is much faster than FaceTools::SurfaceMesher even with cleaning and hole filling afterwards.
    // TODO, fix edge setting so cleaning and hole filling not needed!
    { rlib::CpuTimer cputimer( "[INFO] FaceTools::FaceAverager::init: Model meshing", std::cerr);
        boost::unordered_map<int,IntSet> sedges;
        remesher->createSaddleEdges( sedges);
        RFeatures::ObjModelEdgeFaceAdder edgeAdder( _model);
        edgeAdder.addFaces( sedges);
    }   // end timed section
    delete remesher;

    { rlib::CpuTimer cputimer( "[INFO] FaceTools::FaceAverager::init: Cleaning sampled model", std::cerr);
        FaceTools::clean( _model);
        RFeatures::ObjModelHoleFiller::fillHoles( _model);
        FaceTools::clean( _model);
    }   // end timed section
    // Is now a triangulated manifold

    const int n = _cmat.cols;
    _cmat = cv::Mat_<cv::Vec3f>(1, n);  // n correspondence points (columns)
    // Copy in the sampled vertex locations
    cv::Vec3f* crow = _cmat.ptr<cv::Vec3f>( 0);
    const IntSet& vidxs = _model->getVertexIds();
    //assert( vidxs.size() == (size_t)n);
    int i = 0;
    BOOST_FOREACH ( int vidx, vidxs)
        crow[i++] = _model->vtx( vidx);
}   // end init

/*
size_t FaceAverager::getLandmarks( std::vector<cv::Vec3f>& lmks) const
{
    typedef std::pair<std::string, int> LMPair;
    BOOST_FOREACH ( const LMPair& lmpair, _lmks)
        lmks.push_back( _model->vtx( lmpair.second));
    return _lmks.size();
}   // end getLandmarks
*/
