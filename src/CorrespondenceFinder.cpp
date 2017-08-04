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

#include <CorrespondenceFinder.h>
#include <ObjModelTools.h>
#include <FaceTools.h>
#include <ProgressDelegate.h>       // rlib
using FaceTools::CorrespondenceFinder;
using FaceTools::ObjMetaData;
using FaceTools::RegistrationFace;
using RFeatures::ObjModelKDTree;
using RFeatures::ObjModelCurvatureMap;
using RFeatures::ObjModelSurfacePatches;
using RFeatures::ObjModelAligner;
using RFeatures::ObjModel;
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <boost/foreach.hpp>
//#include <boost/thread.hpp>
//#include <boost/bind.hpp>


// For boost::shared_ptr
class CorrespondenceFinder::Deleter
{ public:
    void operator()( CorrespondenceFinder* cf) const { delete cf;}
};  // end class


// public static
CorrespondenceFinder::Ptr CorrespondenceFinder::create( const ObjMetaData::Ptr omd, int nc, bool makeUniform)
{
    return Ptr( new CorrespondenceFinder( omd, nc, makeUniform), Deleter());
}   // end create


// private
CorrespondenceFinder::CorrespondenceFinder( const ObjMetaData::Ptr omd, int nc, bool makeUniform)
    : _regFace( new RegistrationFace( omd->getCurvatureMap()))
{
    std::cerr << " [ Sampling reference vertices for registration... ]" << std::endl;
    if ( makeUniform)
        _refobj = _regFace->sampleUniformlyInterpolated( nc);
    else
        _refobj = _regFace->sampleCurvatureVariableInterpolated( nc);
    _cmat = cv::Mat_<cv::Vec3f>( 0, _refobj->getNumVertices());   // Correspondence points (columns)
}   // end ctor


// private
CorrespondenceFinder::~CorrespondenceFinder()
{
    if ( _regFace)
        delete _regFace;
}   // end dtor


// public
void CorrespondenceFinder::findCorrespondences( const ObjModelKDTree::Ptr kdtree)
{
    std::cerr << " [ Correspondence mapping ... ]" << std::endl;
    const ObjModel::Ptr model = kdtree->getObject();
    _cmat.resize( _cmat.rows + 1);  // Make another row for the new model
    cv::Vec3f* crow = _cmat.ptr<cv::Vec3f>(_cmat.rows-1);

    const IntSet& vidxs = _refobj->getVertexIds();
    BOOST_FOREACH ( int vidx, vidxs)
        crow[vidx] = model->vtx( kdtree->find( _refobj->vtx(vidx)));
}   // end findCorrespondences


// public
ObjModel::Ptr CorrespondenceFinder::buildModel( const std::vector<cv::Vec3f>& vtxs) const
{
    assert( vtxs.size() == _cmat.cols);
    if ( vtxs.size() != _cmat.cols)
        return ObjModel::Ptr();

    ObjModel::Ptr model = ObjModel::create();
    const int n = vtxs.size();
    IntSet vset;
    for ( int i = 0; i < n; ++i)
    {
        const int vidx = model->addVertex(vtxs[i]);
        if ( vset.count(vidx))
            std::cerr << "ERROR! Added vertex return vertex ID " << vidx << " already in set." << std::endl;
        if ( vidx != i)
            std::cerr << "ERROR! Non-matching return vertex ID for " << vidx << " != " << i << std::endl;
        vset.insert(vidx);
    }   // end for

    const boost::unordered_map<int,IntSet>& edges = _regFace->getEdges();

    typedef std::pair<int,IntSet> ESet;
    BOOST_FOREACH ( const ESet& eset, edges)
    {
        int x = eset.first;
        BOOST_FOREACH ( int y, eset.second)
        {
            model->setEdge( x, y);
        }   // end foreach
    }   // end foreach

    //RFeatures::ObjModelEdgeFaceAdder efadder(model);
    //efadder.addFaces( edges);
    return model;
}   // end buildModel


/*
cv::Mat_<cv::Vec3f> createSurfacePatch( const ObjModel::Ptr model, const IntSet& vidxs)
{
    cv::Mat_<cv::Vec3f> patch( vidxs.size(), 1);
    int j = 0;
    BOOST_FOREACH ( int vidx, vidxs)
        patch.ptr<cv::Vec3f>(j++)[0] = model->vtx(vidx);
    return patch;
}   // end createSurfacePatch


// private
void CorrespondenceFinder::createDenseSourcePatches( const ObjMetaData::Ptr omd)
{
    const int numPointsPerPatch = _patchPoints;
    const int numHighCurvPoints = _hcmat.cols;

    _highCurvSrcPatches.resize( numHighCurvPoints);

    const ObjModel::Ptr model = omd->getObject();
    const ObjModelSurfacePatches patchGrabber( omd->getKDTree(), _patchRadius);
    const cv::Vec3f* hcmatRow = _hcmat.ptr<cv::Vec3f>(0);
    for ( int i = 0; i < numHighCurvPoints; ++i)
    {
        const cv::Vec3f& v = hcmatRow[i];
        IntSet pset;
        patchGrabber.getPatchVertexIds( v, pset, numPointsPerPatch); // Get surface patch on source model at v
        if ( pset.size() < numPointsPerPatch)
            throw VertexDensityException( "Insufficient points on source model within given radius around sample location!", v);

        assert( (int)pset.size() == numPointsPerPatch);
        _highCurvSrcPatches[i] = createSurfacePatch( model, pset);
    }   // end for
}   // end createDenseSourcePatches


class PairwisePatchFinder
{
    const ObjModelSurfacePatches* _samplePointSetFinder;
    const ObjModelSurfacePatches* _densePointSetFinder;
    const ObjModelSurfacePatches* _patchGrabber;
    const cv::Mat_<cv::Vec3f> _srcPatch;    // The source patch to compare against
    cv::Vec3f _bestLoc;
    double _minBendingEnergy;

    // Find the patch location on the sample model corresponding to srcPatch that has minimum bending energy (using thin plate splines)
    void findBestPatch( const IntSet& sampleSet)
    {
        typedef RFeatures::ObjModelPatchBendingEnergy BendingEnergy;
        const int numPatchPoints = _srcPatch.rows;
        const ObjModel::Ptr denseModel = _patchGrabber->getKDTree()->getObject();
        BOOST_FOREACH ( int svid, sampleSet)
        {
            const cv::Vec3f& pcentre = denseModel->vtx(svid);

            // Get the patch vertices from the dense model
            IntSet pset;
            _patchGrabber->getPatchVertexIds( pcentre, pset, numPatchPoints);
            if ( pset.size() < numPatchPoints)
                throw FaceTools::VertexDensityException( "Insufficient points on match model within given radius around sample location!", pcentre);

            assert( pset.size() == numPatchPoints);
            cv::Mat_<cv::Vec3f> patch = createSurfacePatch( denseModel, pset);

            // Calculate the bending energy between the patch from location svid and the source patch
            const double benergy = BendingEnergy::calcE( patch, _srcPatch) + BendingEnergy::calcE( _srcPatch, patch);
            assert( !cvIsNaN( benergy));
            if ( benergy < _minBendingEnergy)
            {
                _minBendingEnergy = benergy;
                _bestLoc = pcentre;
            }   // end if
        }   // end foreach
    }   // end findBestPatch

public:
    PairwisePatchFinder( const ObjModelSurfacePatches* spsf, const ObjModelSurfacePatches* dpsf, const ObjModelSurfacePatches* patchGrabber,
                         const cv::Mat_<cv::Vec3f> srcPatch, const cv::Vec3f& bestLoc)
        : _samplePointSetFinder(spsf), _densePointSetFinder(dpsf), _patchGrabber(patchGrabber),
          _srcPatch(srcPatch), _bestLoc(bestLoc), _minBendingEnergy( DBL_MAX)
    {}   // end ctor

    void find()
    {
        IntSet sset0;   // Get ALL high curvature points within radius of source patch.
        _samplePointSetFinder->getPatchVertexIds( _bestLoc, sset0);
        // Also include in sset0 the point closest to _bestLoc on the dense model
        // since very sparse samplings of high curvature points may exclude best candidates.
        sset0.insert( _densePointSetFinder->getKDTree()->find( _bestLoc));
        findBestPatch( sset0); // Find the best patch from sset0 vertices

        IntSet sset1; // Search within a more localised region (having radius fineSearchRadius) on the dense model
        _densePointSetFinder->getPatchVertexIds( _bestLoc, sset1);
        BOOST_FOREACH ( int vidx, sset0)    // Don't recheck previously checked vertices
            sset1.erase(vidx);  // Don't want to double check any vertices
        findBestPatch( sset1);
    }   // end find


    const cv::Vec3f& getBestLocation() const { return _bestLoc;}
};  // end class


void threadFunctionPairwisePatchFinder( const std::vector<PairwisePatchFinder*>* ppfinders, rlib::ProgressDelegate* pd)
{
    const int N = (int)ppfinders->size();
    for ( int i = 0; i < N; ++i)
    {
        pd->updateProgress( float(i)/N);
        ppfinders->at(i)->find(); // Blocks
    }   // end foreach
    pd->updateProgress(1.0f);
}   // end threadFunctionPairwisePatchFinder


// private
int CorrespondenceFinder::createReferenceMappings( const ObjMetaData::Ptr omd)
{
    const ObjModel::Ptr model = omd->getObject();
    _hcmat = cv::Mat_<cv::Vec3f>( 1, _f1.size());   // Initially just a single row
    // Set the positions of all the high curvature points
    int i = 0;
    cv::Vec3f* hcmatRow = _hcmat.ptr<cv::Vec3f>(0);
    BOOST_FOREACH ( int svidx, _f1)
        hcmatRow[i++] = model->vtx(svidx);
    // Create and save the high curvature aligner for later use in ICP registration
    createDenseSourcePatches( omd); // Create and store source patches for correspondence matching
}   // end createReferenceMappings
*/


    /*
    const ObjModelSurfacePatches samplePointSetFinder( ObjModelKDTree::create( model, _f1), coarseSearchRadius);
    const ObjModelSurfacePatches densePointSetFinder( omd->getKDTree(), fineSearchRadius);
    const ObjModelSurfacePatches patchGrabber( omd->getKDTree(), _patchRadius);

    _hcmat.resize( _hcmat.rows + 1);  // Make another row for the new model
    const cv::Vec3f* hcmatSrcRow = _hcmat.ptr<cv::Vec3f>(0);
    cv::Vec3f* hcmatNewRow = _hcmat.ptr<cv::Vec3f>( _hcmat.rows - 1);   // Pointer to the row we just added

    // Create the processing threads
    const int NUM_THREADS = boost::thread::hardware_concurrency();
    boost::thread_group* tgroup = new boost::thread_group;
    std::vector< std::vector<PairwisePatchFinder*>* > threadPairwisePatchFinders(NUM_THREADS);
    for ( int i = 0; i < NUM_THREADS; ++i)
        threadPairwisePatchFinders[i] = new std::vector<PairwisePatchFinder*>;

    // Asign pairwise matcher objects to each of the processing data structures
    const int numHighCurvPoints = _hcmat.cols;
    for ( int i = 0; i < numHighCurvPoints; ++i)
    {
        const cv::Mat_<cv::Vec3f>& srcPatch = _highCurvSrcPatches[i]; // The source patch to compare against
        const cv::Vec3f& bestLoc = hcmatSrcRow[i];    // The centre of the search radius for a matching patch (same initial location as source)
        PairwisePatchFinder* ppf = new PairwisePatchFinder( &samplePointSetFinder, &densePointSetFinder, &patchGrabber, srcPatch, bestLoc);
        threadPairwisePatchFinders[ i % NUM_THREADS]->push_back(ppf);
    }   // end for

    // Start the threads
    std::ostringstream ossPrefix;
    ossPrefix << " [ Correspondence matching high curvature points across " << NUM_THREADS << " CPU threads - please wait... (";
    rlib::OsPcntUpdater pcntUpdater( &std::cerr, NUM_THREADS, ossPrefix.str(), std::string(") ]"));
    for ( int i = 0; i < NUM_THREADS; ++i)
        tgroup->create_thread( boost::bind( threadFunctionPairwisePatchFinder, threadPairwisePatchFinders[i], &pcntUpdater));

    tgroup->join_all();
    delete tgroup;
    std::cerr << std::endl;

    // Collect the responses
    for ( int i = numHighCurvPoints-1; i >= 0; --i)
    {
        const PairwisePatchFinder* ppf = threadPairwisePatchFinders[ i % NUM_THREADS]->back();
        threadPairwisePatchFinders[ i % NUM_THREADS]->pop_back();
        hcmatNewRow[i] = ppf->getBestLocation();  // Set bestLoc as location on the model that best matches the source patch
        delete ppf;
    }   // end for

    // Release mem
    for ( int i = 0; i < NUM_THREADS; ++i)
    {
        std::vector<PairwisePatchFinder*>* ppfs = threadPairwisePatchFinders[i];
        assert(ppfs->empty());
        delete ppfs;
    }   // end for
    */
