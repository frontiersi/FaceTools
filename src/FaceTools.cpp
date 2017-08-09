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
using RFeatures::ObjModel;
using RFeatures::CameraParams;
#include <Landmarks.h>


void checkBoundaries( const RFeatures::ObjModelBoundaryFinder& boundaryFinder)
{
    const int nb = boundaryFinder.getNumBoundaries();
    std::cerr << "\tFound " << nb << " boundaries of lengths:" << std::endl;
    for ( int i = 0; i < nb; ++i)
    {
        const std::list<int>& blist = boundaryFinder.getBoundary(i);
        std::cerr << "\t" << i << ") " << blist.size() << " :";
        // Check that the boundaries do not contain duplicate indices
        IntSet bset( blist.begin(), blist.end());
        assert( bset.size() == blist.size());
        // Print the list of vertices
        if ( blist.size() < 25)
        {
            BOOST_FOREACH ( const int& uv, blist)
                std::cerr << " " << uv;
        }   // end if
        else
            std::cerr << " TOO LONG TO PRINT";
        std::cerr << std::endl;
    }   // end for
}   // end checkBoundaries



cv::Matx44d FaceTools::orient( ObjMetaData::Ptr omd)
{
    using namespace FaceTools;
    if ( omd->getKDTree() == NULL)
        omd->rebuildKDTree();
    if ( omd->getCurvatureMap() == NULL)
    {
        const cv::Vec3f& ntip = omd->getLandmark( Landmarks::NASAL_TIP);
        omd->rebuildCurvatureMap( omd->getKDTree()->find( ntip));
    }   // end if

    const cv::Vec3f v0 = omd->getLandmark( Landmarks::L_EYE_CENTRE);
    const cv::Vec3f v1 = omd->getLandmark( Landmarks::R_EYE_CENTRE);

    FaceTools::FaceOrienter orienter( omd->getCurvatureMap(), omd->getKDTree());
    cv::Vec3d nvec, uvec;
    orienter( v0, v1, nvec, uvec);
    // Get the complementary axes for rotation
    const cv::Vec3d cnvec = cv::Vec3d(0,0,2) - nvec;
    const cv::Vec3d cuvec = cv::Vec3d(0,2,0) - uvec;

    const RFeatures::ObjModelMover rmat( cnvec, cuvec);
    omd->transform( rmat());
    return rmat();  // Return the matrix used to do the rotation
}   // end orient



ObjModel::Ptr FaceTools::getComponent( const ObjModel::Ptr model, int svid, const cv::Vec3d& coffset)
{
    using namespace RFeatures;

    ObjModelTriangleMeshParser parser( model);
    ObjModelBoundaryFinder boundaryFinder( model);
    parser.setBoundaryParser( &boundaryFinder);

    const IntSet& sfids = model->getFaceIds( svid);
    assert( !sfids.empty());
    parser.parse( *sfids.begin());

    boundaryFinder.sortBoundaries();    // Sort the discovered boundaries (max length first)
    //checkBoundaries( boundaryFinder);
    const std::list<int>& blist = boundaryFinder.getBoundary(0);
    boundaryFinder.reset( &blist);   // Reset with boundary of the component we want (max number of edges)

    const ObjModelMover mover( coffset);
    ObjModelCopier modelCopier( model, &mover);
    parser.reset();
    parser.setBoundaryParser( &boundaryFinder);
    parser.addTriangleParser( &modelCopier);

    // There can only be one polygon that shares and two consecutive vertices in blist.
    // Get a polygon ID in this manner to ensure that the starting polygon for parsing
    // is inside the blist boundary.
    const IntSet& sfids2 = model->getSharedFaces( *blist.begin(), *(++blist.begin()));
    parser.parse( *sfids2.begin());

    return modelCopier.getCopiedModel();
}   // end getComponent



ObjModel::Ptr FaceTools::crop( const ObjModel::Ptr m, const cv::Vec3f& v, double radius, const cv::Vec3d& offset)
{
    RFeatures::ObjModelCropper cropper( m, v, radius);
    const RFeatures::ObjModelMover mover( offset);
    RFeatures::ObjModelCopier copier(m, &mover);

    RFeatures::ObjModelTriangleMeshParser parser(m);
    parser.setBoundaryParser( &cropper);
    parser.addTriangleParser( &copier);

    // Start parsing at the vertex closest to v.
    int startVidx = 0;
    double maxd = DBL_MAX;
    double d;
    const IntSet& vidxs = m->getVertexIds();
    BOOST_FOREACH ( int vidx, vidxs)
    {
        d = cv::norm( m->getVertex(vidx) - v);
        if ( d < maxd)
        {
            startVidx = vidx;
            maxd = d;
        }   // end if
    }   // end foreach
    const int sfid = *m->getFaceIds(startVidx).begin(); // Starting face
    parser.parse(sfid);

    ObjModel::Ptr nmodel = copier.getCopiedModel();

    // Finally, remove vertices (and attached faces) that connect to 2 or fewer polygons so the boundary is clean.
    RFeatures::ObjModelCleaner cleaner( nmodel);
    cleaner.remove3D();
    cleaner.remove1D();
    int pruned = 0;
    int totPruned = 0;
    do
    {
        pruned = cleaner.pruneVertices(2);
        totPruned += pruned;
    } while ( pruned > 0);

    return nmodel;
}   // end crop


cv::Vec3f FaceTools::calcFaceCentre( const cv::Vec3f& v0, const cv::Vec3f& v1, const cv::Vec3f& nt)
{
    return cv::Vec3f( (v0[0] + v1[0] + nt[0])/3, nt[1], (v0[2] + v1[2])/2);
}   // end calcFaceCentre


cv::Vec3f FaceTools::calcFaceCentre( const ObjMetaData::Ptr omd)
{
    const cv::Vec3f& nt = omd->getLandmark("nasal_tip");
    const cv::Vec3f& v0 = omd->getLandmark("l_eye_centre");
    const cv::Vec3f& v1 = omd->getLandmark("r_eye_centre");
    return calcFaceCentre( v0, v1, nt);
}   // end calcFaceCentre


ObjModel::Ptr FaceTools::cropAroundFaceCentre( const ObjMetaData::Ptr omd, double G)
{
    const cv::Vec3f& v0 = omd->getLandmark("l_eye_centre");
    const cv::Vec3f& v1 = omd->getLandmark("r_eye_centre");
    const cv::Vec3f fcentre = calcFaceCentre(omd);
    const double cropRadius = G * (cv::norm( fcentre - v0) + cv::norm( fcentre - v1))/2;
    return crop( omd->getObject(), fcentre, cropRadius);
}   // end cropAroundFaceCentre


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
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    BOOST_FOREACH ( int vidx, vidxs)
        omod->addVertex( smod->vtx(vidx));
    return omod;
}   // end createFromSubset


ObjModel::Ptr FaceTools::createFromTransformedSubset( const ObjModel::Ptr smod, const IntSet& vidxs, const cv::Matx44d& T,
                                                      boost::unordered_map<int,int>* newVidxsToSource)
{
    const RFeatures::ObjModelMover transformer(T);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    BOOST_FOREACH ( int vidx, vidxs)
    {
        cv::Vec3f v = smod->vtx(vidx);
        transformer( v);
        const int nvidx = omod->addVertex(v);
        if ( newVidxsToSource)
            (*newVidxsToSource)[nvidx] = vidx;
    }   // end foreach
    return omod;
}   // end createFromTransformedSubset


// Flatten m to XY plane and return it, also setting fmap to be the
// vertex ID mapping from the returned flattened object to the original object m.
ObjModel::Ptr FaceTools::makeFlattened( const ObjModel::Ptr m, boost::unordered_map<int,int>* fmap)
{
    if ( fmap)
        fmap->clear();
    const IntSet& vidxs = m->getVertexIds();
    int nvidx;
    ObjModel::Ptr fmod = ObjModel::create( m->getSpatialPrecision());
    BOOST_FOREACH ( int vidx, vidxs)
    {
        cv::Vec3f v = m->vtx(vidx);
        v[2] = 0;
        nvidx = fmod->addVertex(v);
        if ( fmap)
            (*fmap)[nvidx] = vidx;
    }   // end foreach
    // fmod is now a flattened version
    return fmod;
}   // end makeFlattened


void FaceTools::clean( ObjModel::Ptr model)
{
    RFeatures::ObjModelCleaner omc(model);
    const int rem3d = omc.remove3D();
    const int rem1d = omc.remove1D();
    int totRemTV = 0;
    int remTV = 0;
    do
    {
        remTV = RFeatures::ObjModelTetrahedronReplacer( model).removeTetrahedrons();
        totRemTV += remTV;
    } while ( remTV > 0);
    std::cerr << "Removed " << rem3d << " 3D vertices" << std::endl;
    std::cerr << "Removed " << rem1d << " 1D vertices" << std::endl;
    std::cerr << "Removed/replaced " << totRemTV << " tetrahedron peaks" << std::endl;
}   // end clean


int FaceTools::fillHoles( ObjModel::Ptr model)
{
    using namespace RFeatures;

    ObjModelTriangleMeshParser parser( model);
    ObjModelBoundaryFinder boundaryFinder( model);
    parser.setBoundaryParser( &boundaryFinder);

    parser.parse();

    boundaryFinder.sortBoundaries();    // Sort the discovered boundaries (max length first)
    const int nboundaries = (int)boundaryFinder.getNumBoundaries();

    // Don't fill the largest boundary since this is the outer boundary!
    for ( int i = 1; i < nboundaries; ++i)
    {
        const std::list<int>& blist = boundaryFinder.getBoundary(i);

        const int firstVtx = *blist.begin();
        std::list<int>::const_iterator ij = ++blist.begin();
        std::list<int>::const_iterator ik = ++(++blist.begin());
        for ( ; ik != blist.end(); ++ij, ++ik)
        {
            assert( *ij != *ik);
            model->setFace( firstVtx, *ij, *ik);
        }   // end for
    }   // end for

    ObjModelCleaner omc(model);
    omc.remove3D();
    omc.remove1D();

    /*
    // Get the component attached to the original largest boundary
    boundaryFinder.reset();
    parser.parse();
    boundaryFinder.sortBoundaries();

    const std::list<int>& bverts = boundaryFinder.getBoundary(0);
    const IntSet& sfids = model->getFaceIds( *bverts.begin());  // All face IDs connected to the first boundary vertex
    const int sfid = *model->getFaceIds( *bverts.begin()).begin();    // Start face on external boundary (save here since resetting).
    boundaryFinder.reset( &bverts); // bverts is "hanging" alias after this!
    ObjModelCopier copier( model);
    parser.addTriangleParser(&copier);
    parser.parse( sfid);
    model = copier.getCopiedModel();
    */

    return nboundaries;
}   // end fillHoles



int FaceTools::collapseSmallPolygons( ObjModel::Ptr model, double minArea)
{
    using namespace RFeatures;
    ObjModelTriangleMeshParser parser( model);
    ObjModelPolygonAreaCalculator polyAreaCalculator( model);
    parser.addTriangleParser( &polyAreaCalculator);
    parser.parse();

    ObjModelTopologyFinder topFinder( model);
    int totCount = 0;

    int count = 0;
    do
    {
        count = 0;
        const IntSet& fids = parser.getParsedFaces();
        std::cerr << "Num polys parsed = " << fids.size() << std::endl;
        std::cerr << "Num polys in model = " << model->getNumFaces() << std::endl;
        BOOST_FOREACH ( int fid, fids)
        {
            // Collapsing a previous polygon may have also removed this one
            if ( !model->getFaceIds().count(fid))
                continue;

            /*
            // Don't collapse boundary polygons
            const ObjPoly& poly = model->poly(fid);
            if ( topFinder.isBoundary(poly.vindices[0]) || topFinder.isBoundary(poly.vindices[1]) || topFinder.isBoundary(poly.vindices[2]))
                continue;
            */

            if ( polyAreaCalculator.getPolygonArea(fid) < minArea)
            {
                ObjModelPolygonCollapser( model).collapse(fid);
                count++;
            }   // end if
        }   // end foreach
        totCount += count;
    } while ( count > 0);

    return totCount;
}   // end collapseSmallPolygons
