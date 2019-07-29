/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <CurvatureMetricCalculatorType.h>
#include <ObjModelCurvatureMetrics.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FeatureUtils.h>   // RFeatures
#include <FaceModelCurvature.h>
#include <sstream>
using FaceTools::Metric::CurvatureMetricCalculatorType;
using FaceTools::Metric::MCT;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::LmkList;
using FaceTools::FM;


CurvatureMetricCalculatorType::CurvatureMetricCalculatorType( int id, const LmkList* l0, const LmkList* l1)
    : _vis( id, l0, l1) {}


MCT* CurvatureMetricCalculatorType::make( int id, const LmkList* l0, const LmkList* l1) const
{
    return new CurvatureMetricCalculatorType(id, l0, l1);
}   // end make


bool CurvatureMetricCalculatorType::canCalculate( const FM* fm, int aid, const LmkList* ll) const
{
    if ( FaceModelCurvature::rmetrics(fm) == nullptr)
        return false;
    using SLmk = FaceTools::Landmark::SpecificLandmark;
    const LandmarkSet& lmks = fm->assessment(aid)->landmarks();
    return std::all_of( std::begin(*ll), std::end(*ll), [&lmks]( const SLmk& p){ return lmks.has(p);});
}   // end canCalculate


namespace  {

struct RegionParser : public RFeatures::ObjModelBoundaryParser
{
    explicit RegionParser( const IntSet& bset) : _bset(bset) {}
    ~RegionParser() override {}

    bool parseEdge( int fid, const cv::Vec2i& e, int&) override
    {
        const int v0 = e[0];
        const int v1 = e[1];
        if ( _bset.count(v0) == 0 || _bset.count(v1) == 0)
            return true;

        const int fid2 = model->oppositePoly( fid, v0, v1);
        return fid2 >= 0 && _bset.count( model->face(fid2).opposite(v0,v1)) > 0;
    }   // end parseEdge

private:
    const IntSet &_bset;   // Set of boundary vertices
};  // end struct

}   // end namespace


void CurvatureMetricCalculatorType::measure( std::vector<double>& dvals, const FM* fm, int aid, const LmkList* ll) const
{
    const LandmarkSet& lmks = fm->assessment(aid)->landmarks();
    const RFeatures::ObjModel& model = fm->model();

    IntSet bset;
    std::vector<int> pvids;
    RFeatures::DijkstraShortestPathFinder dspf( model);

    cv::Vec3f mpos(0,0,0);  // For finding the seed vertex (polygon) for parsing the region

    // Find all the vertices on the boundary of the demarcated region
    const auto* pp = &*ll->rbegin();        // Previous point
    for ( const auto& tp : *ll)
    {
        const cv::Vec3f v0 = lmks.pos( *pp);
        const cv::Vec3f v1 = lmks.pos( tp);
        mpos += v1; // Get the mean position to seed mesh parsing from

        // Get the closest vertices for finding a shortest path along connecting edges
        const int vtx0 = fm->findVertex( v0);
        const int vtx1 = fm->findVertex( v1);

        dspf.setEndPointVertexIndices( vtx1, vtx0);
        dspf.findShortestPath( pvids);
        bset.insert(std::begin(pvids), std::end(pvids));    // Add to the set of boundary vertices

        pp = &tp;
    }   // end for

    // Get the seed polygon for parsing within the demarcated region
    mpos *= 1.0f/ll->size();
    const int seedVtx = fm->findVertex( mpos);
    const int seedPoly = *model.faces(seedVtx).begin();

    // Parse the given region and collect the set of parsed polygon ids
    RegionParser bparser( bset);
    RFeatures::ObjModelTriangleMeshParser parser( model);
    parser.setBoundaryParser(&bparser);
    parser.parse( seedPoly);
    const IntSet& pfaces = parser.parsed();

    dvals.resize(4, 0);
    if ( pfaces.empty())
    {
        std::cerr << "[WARN] FaceTools::Metric::CurvatureMetricCalculatorType::measure: No polygons parsed!" << std::endl;
        return;
    }   // end if

    // Now, for each of the parsed faces, collect the required statistic
    double v0 = 0.0;
    double v1 = 0.0;
    double v2 = 0.0;
    double v3 = 0.0;

    {
        FaceModelCurvature::RPtr sm = FaceModelCurvature::rmetrics(fm);
        RFeatures::ObjModelCurvatureMetrics metrics( model, fm->manifolds(), *sm);
        for ( int fid : pfaces)
        {
            const double kp1 = metrics.faceKP1FirstOrder(fid);
            const double kp2 = metrics.faceKP2FirstOrder(fid);
            v0 += kp1;
            v1 += kp2;
            v2 += kp1 + kp2;    // Mean curvature (/2 factored out)
            v3 += kp1 * kp2;    // Gaussian curvature
        }   // end for
    }

    dvals[0] = v0/pfaces.size();
    dvals[1] = v1/pfaces.size();
    dvals[2] = 0.5 * v2/pfaces.size();
    dvals[3] = v3/pfaces.size();
}   // end measure
