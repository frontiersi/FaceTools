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

#include <FaceTools.h>
#include <algorithm>
#include <FaceShapeLandmarks2DDetector.h>   // namespace FaceTools::Landmarks
#include <AssetImporter.h>                  // RModelIO
#include <ObjModelBoundaryFinder2.h>        // RFeatures
#include <ObjModelTriangleMeshParser.h>     // RFeatures
#include <ObjModelRegionSelector.h>         // RFeatures
#include <ObjModelTetrahedronReplacer.h>    // RFeatures
#include <ObjModelIntegrityChecker.h>       // RFeatures
#include <ObjModelCleaner.h>                // RFeatures
#include <ObjModelCopier.h>                 // RFeatures
using namespace RFeatures;


int FaceTools::findBoundaryLoops( const ObjModel::Ptr model, std::list<std::vector<cv::Vec3f> > &loops)
{
    ObjModelBoundaryFinder2 bfinder(model);
    const int nbs = bfinder.findOrderedBoundaryVertices();
    bfinder.sortBoundaries(true);

    for ( int i = 0; i < nbs; ++i)
    {
        const std::list<int>& blist = bfinder.getBoundary(i);
        //assert( model->getConnectedVertices( blist.back()).count( blist.front()) > 0); // Boundary start must join end
        //std::cerr << "  + boundary has " << blist.size() << " vertices" << std::endl;
        loops.resize( loops.size() + 1);
        std::vector<cv::Vec3f>& lvec = loops.back();
        std::for_each( std::begin( blist), std::end(blist), [&](int b){ lvec.push_back( model->vtx(b));});
    }   // end for
    return nbs;
}   // end findBoundaryLoops


ObjModel::Ptr FaceTools::getComponent( const ObjModel::Ptr model, int svidx)
{
    if ( model->getVertexIds().count(svidx) == 0)
    {
        std::cerr << "[ERROR] FaceTools::getComponent: Invalid starting vertex!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    ObjModelTriangleMeshParser parser( model);
    parser.parse( *model->getFaceIds( svidx).begin());
    const IntSet& fids = parser.getParsedFaces();
    ObjModelCopier copier( model);
    std::for_each( std::begin(fids), std::end(fids), [&](int fid){ copier.addTriangle(fid);});
    return copier.getCopiedModel();
}   // end getComponent


ObjModel::Ptr FaceTools::crop( const ObjModel::Ptr model, const cv::Vec3f& v, int svidx, double radius)
{
    if ( model->getVertexIds().count(svidx) == 0)
    {
        std::cerr << "[ERROR] FaceTools::crop: Invalid starting vertex!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    if ( model->getFaceIds( svidx).empty())
    {
        std::cerr << "[ERROR] FaceTools::crop: Given start vertex not used in any of the model's polygons!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    ObjModelRegionSelector::Ptr cropper = ObjModelRegionSelector::create( model, v, svidx);
    cropper->setRadius( radius);
    IntSet cfids;
    cropper->getRegionFaces( cfids);
    assert( !cfids.empty());
    return crop( model, cfids);
}   // end crop


ObjModel::Ptr FaceTools::crop( const ObjModel::Ptr model, const IntSet& cfids)
{
    assert( !cfids.empty());
    // Copy the subset of faces into a new model
    ObjModelCopier copier( model);
    std::for_each( std::begin(cfids), std::end(cfids), [&](int fid){ copier.addTriangle(fid);});
    ObjModel::Ptr cmodel = copier.getCopiedModel();

    // Remove vertices (and attached faces) that connect to 2 or fewer polygons so the boundary is clean.
    ObjModelCleaner cleaner( cmodel);
    cleaner.remove3D();
    cleaner.remove1D();
    int pruned = 0;
    int totPruned = 0;
    do
    {
        pruned = cleaner.pruneVertices(2);
        totPruned += pruned;
    } while ( pruned > 0);

    return getComponent( cmodel, *cmodel->getVertexIds().begin());
}   // end crop


bool FaceTools::hasReqLandmarks( const LandmarkSet& lset)
{
    using namespace FaceTools::Landmarks;
    return lset.has( L_EYE_CENTRE) && lset.has(R_EYE_CENTRE) && lset.has(NASAL_TIP);
}   // end hasReqLandmarks


cv::Vec3f FaceTools::calcFaceCentre( const cv::Vec3f& upv, const cv::Vec3f& v0, const cv::Vec3f& v1, const cv::Vec3f& nt)
{
    const cv::Vec3f midEye = (v0 + v1) * 0.5;
    const cv::Vec3f dvec = nt - midEye;
    cv::Vec3f downVec;
    cv::normalize( -upv, downVec);
    return midEye + (downVec.dot(dvec) * downVec);
}   // end calcFaceCentre


double FaceTools::calcFaceCropRadius( const cv::Vec3f& fcentre, const cv::Vec3f& v0, const cv::Vec3f& v1, double G)
{
    return G * (cv::norm( fcentre - v0) + cv::norm( fcentre - v1))/2;
}   // end calcFaceCropRadius


ObjModel::Ptr FaceTools::createFromVertices( const cv::Mat_<cv::Vec3f>& vrow)
{
    ObjModel::Ptr omod = ObjModel::create();
    const int npoints = vrow.cols;
    const cv::Vec3f* vptr = vrow.ptr<cv::Vec3f>(0);
    for ( int i = 0; i < npoints; ++i)
        omod->addVertex( vptr[i]);
    return omod;
}   // end createFromVertices


ObjModel::Ptr FaceTools::createFromSubset( const ObjModel::Ptr smod, const IntSet& vidxs)
{
    assert(smod);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    std::for_each( std::begin(vidxs), std::end(vidxs), [&](int vidx){ omod->addVertex( smod->vtx(vidx));});
    return omod;
}   // end createFromSubset


ObjModel::Ptr FaceTools::createFromTransformedSubset( const ObjModel::Ptr smod, const IntSet& vidxs, const cv::Matx44d& T,
                                                      std::unordered_map<int,int>* newVidxsToSource)
{
    assert(smod);
    const RFeatures::ObjModelMover transformer(T);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    for ( int vidx : vidxs)
    {
        cv::Vec3f v = smod->vtx(vidx);
        transformer( v);
        const int nvidx = omod->addVertex(v);
        if ( newVidxsToSource)
            (*newVidxsToSource)[nvidx] = vidx;
    }   // end for
    return omod;
}   // end createFromTransformedSubset


// Flatten m to XY plane and return it, also setting fmap to be the
// vertex ID mapping from the returned flattened object to the original object m.
ObjModel::Ptr FaceTools::makeFlattened( const ObjModel::Ptr m, std::unordered_map<int,int>* fmap)
{
    assert(m);
    if ( fmap)
        fmap->clear();
    const IntSet& vidxs = m->getVertexIds();
    int nvidx;
    ObjModel::Ptr fmod = ObjModel::create( m->getSpatialPrecision());
    for ( int vidx : vidxs)
    {
        cv::Vec3f v = m->vtx(vidx);
        v[2] = 0;
        nvidx = fmod->addVertex(v);
        if ( fmap)
            (*fmap)[nvidx] = vidx;
    }   // end for
    // fmod is now a flattened version
    return fmod;
}   // end makeFlattened


void FaceTools::clean( ObjModel::Ptr model)
{
    assert(model);
    RFeatures::ObjModelCleaner omc(model);
    const int rem3d = omc.remove3D();
    const int rem1d = omc.remove1D();
    if ( rem3d > 0 || rem1d > 0)
        std::cerr << "[INFO] FaceTools::clean: Removed " << rem3d << " 3D and " << rem1d << " 1D vertices" << std::endl;
    int totRemTV = 0;
    int remTV = 0;
    do
    {
        remTV = RFeatures::ObjModelTetrahedronReplacer( model).removeTetrahedrons();
        totRemTV += remTV;
    } while ( remTV > 0);
    if ( totRemTV > 0)
        std::cerr << "[INFO] FaceTools::clean: Removed/replaced " << totRemTV << " tetrahedron peaks" << std::endl;
}   // end clean


ObjModel::Ptr FaceTools::loadModel( const std::string& fname, bool useTexture, bool doClean)
{
    RModelIO::AssetImporter assetImporter( useTexture);  // Load textures if selected
    const std::string fext = getExtension(fname);
    // If the file extension is not supported, return NULL.
    if ( assetImporter.getAvailable().count(fext) == 0)
        return ObjModel::Ptr();

    if ( !assetImporter.enableFormat(fext))
    {
        std::cerr << "[ERROR] FaceTools::loadModel: Error enabling RModelIO::AssetImporter format!" << std::endl;
        return ObjModel::Ptr();
    }   // end if

    std::cerr << " =====[ Loading Model '" << fname << "' ]=====" << std::endl;
    ObjModel::Ptr model = assetImporter.load( fname);
    if ( !model)
        std::cerr << "Unable to read in object from '" << fname << "'!" << std::endl;
    else
    {
        // Merge materials if more than one - don't want multiple textures (or meshes)
        if ( model->getNumMaterials() > 1)
        {
            std::cerr << " =====[ Combining Textures ]====="<< std::endl;
            model->mergeMaterials();
            assert( model->getNumMaterials() == 1);
        }   // end if

        if ( doClean)
        {
            RFeatures::ObjModelIntegrityChecker ic( model);
            ic.checkIntegrity();
            if ( !ic.is2DManifold())
            {
                std::cerr << " =====[ Cleaning Model ]====="<< std::endl;
                FaceTools::clean(model);
                ic.checkIntegrity();
                assert( ic.is2DManifold());
                if ( !ic.is2DManifold())
                {
                    std::cerr << "[ERROR] FaceTools::loadModel: Can't make triangulated mesh from \""
                              << fname << "\"" << std::endl;
                    model.reset();
                }   // end if
            }   // end if
        }   // end if
    }   // end if

    return model;
}   // end loadModel
