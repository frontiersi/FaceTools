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

#include <FaceTools.h>
#include <FaceShapeLandmarks2DDetector.h>
#include <FaceModelViewer.h>
#include <LandmarksManager.h>
#include <FaceModel.h>
#include <Transformer.h>        // RFeatures
#include <ImageGrabber.h>       // RVTK
#include <VtkActorCreator.h>    // RVTK
#include <algorithm>
#include <FaceView.h>
using namespace RFeatures;
using namespace FaceTools::Landmark;


bool FaceTools::hasCentreLandmarks( const LandmarkSet& lmks)
{
    using namespace FaceTools::Landmark;
    return lmks.hasCode( SN) && // subnasale
           lmks.hasCode( EX) && // lateral canthi
           lmks.hasCode( EN) && // medial canthi
           lmks.hasCode( PS) && // palpebral superius
           lmks.hasCode( PI);   // palpebral inferius
}   // end hasCentreLandmarks


cv::Vec3f FaceTools::calcPupil( const LandmarkSet& lmks, FaceLateral lat)
{
    assert( lat == FACE_LATERAL_LEFT || lat == FACE_LATERAL_RIGHT);
    cv::Vec3f v0 = *lmks.pos( EX, lat)
                 + *lmks.pos( EN, lat)
                 + *lmks.pos( PS, lat)
                 + *lmks.pos( PI, lat);
    v0 *= 1.0f/4;
    return v0;
}   // end calcPupil


cv::Vec3f FaceTools::calcFaceCentre( const LandmarkSet& lmks)
{
    cv::Vec3f v0 = calcPupil( lmks, FACE_LATERAL_LEFT);
    cv::Vec3f v1 = calcPupil( lmks, FACE_LATERAL_RIGHT);
    return 0.25f * (v0 + v1) + 0.5f * *lmks.pos( SN);
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


ObjModel::Ptr FaceTools::createFromSubset( const ObjModel* smod, const IntSet& vidxs)
{
    assert(smod);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    std::for_each( std::begin(vidxs), std::end(vidxs), [&](int vidx){ omod->addVertex( smod->vtx(vidx));});
    return omod;
}   // end createFromSubset


ObjModel::Ptr FaceTools::createFromTransformedSubset( const ObjModel* smod, const IntSet& vidxs, const cv::Matx44d& T,
                                                      std::unordered_map<int,int>* newVidxsToSource)
{
    assert(smod);
    const RFeatures::Transformer transformer(T);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    for ( int vidx : vidxs)
    {
        cv::Vec3f v = smod->vtx(vidx);
        transformer.transform( v);
        const int nvidx = omod->addVertex(v);
        if ( newVidxsToSource)
            (*newVidxsToSource)[nvidx] = vidx;
    }   // end for
    return omod;
}   // end createFromTransformedSubset


// Flatten m to XY plane and return it, also setting fmap to be the
// vertex ID mapping from the returned flattened object to the original object m.
ObjModel::Ptr FaceTools::makeFlattened( const ObjModel* m, std::unordered_map<int,int>* fmap)
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


RVTK::Viewer::Ptr FaceTools::makeOffscreenViewer( size_t sqdims, float rng, const ObjModel* model)
{
    // Initialise the viewer
    RVTK::Viewer::Ptr viewer = RVTK::Viewer::create(true/*offscreen*/);
    viewer->setSize( sqdims, sqdims);
    viewer->setCamera( RFeatures::CameraParams( cv::Vec3f( 0, 0, rng)));    // Default camera for detecting orientation
    viewer->renderer()->UseFXAAOn();

    // Create the actor
    vtkSmartPointer<vtkTexture> texture;
    RVTK::VtkActorCreator ac;
    vtkSmartPointer<vtkActor> actor = ac.generateActor( model, texture);

    // Add the actor to the viewer
    viewer->addActor( actor);
    viewer->resetClippingRange();
    viewer->updateRender();
    return viewer;
}   // end createOffscreenViewer


cv::Mat_<cv::Vec3b> FaceTools::snapshot( RVTK::Viewer::Ptr v)
{
    RVTK::ImageGrabber ig(*v);
    return ig.colour();
}   // end snapshot


void FaceTools::updateRenderers( const FMS& fms)
{
    FMVS fmvs;
    for ( FM* fm : fms)
    {
        for ( Vis::FV* fv : fm->fvs())
            fmvs.insert(fv->viewer());
    }   // end for
    std::for_each( std::begin(fmvs), std::end(fmvs), [](FMV* fmv){ fmv->updateRender();});
}   // end updateRenderers


cv::Vec3f FaceTools::toSurface( const ObjModelKDTree* kdt, const cv::Vec3f& v)
{
    int notused;
    cv::Vec3f fv;
    const RFeatures::ObjModelSurfacePointFinder spfinder( kdt->model());
    int vidx = kdt->find(v);
    spfinder.find( v, vidx, notused, fv);
    return fv;
}   // end toSurface


cv::Vec3f FaceTools::moveTo( const ObjModelKDTree* kdt, const cv::Vec3f& s, const cv::Vec3f& v)
{
    int notused;
    cv::Vec3f fv;
    const RFeatures::ObjModelSurfacePointFinder spfinder( kdt->model());
    int vidx = kdt->find(s);
    spfinder.find( v, vidx, notused, fv);
    return fv;
}   // end moveTo


cv::Vec3f FaceTools::findDeepestPoint( const ObjModelKDTree* kdt, const cv::Vec3f& p0, const cv::Vec3f& p1, double *dout)
{
    using namespace RFeatures;
    const ObjModel *model = kdt->model();

    DijkstraShortestPathFinder spfinder( kdt->model());
    const int v0 = kdt->find(p0);
    const int v1 = kdt->find(p1);
    spfinder.setEndPointVertexIndices( v0, v1);
    std::vector<int> vpids;
    spfinder.findShortestPath( vpids);

    cv::Vec3f u;
    cv::normalize( p1 - p0, u);

    double maxd = 0;
    int bv = v0;
    const size_t n = vpids.size();
    for ( size_t i = 0; i < n; ++i)
    {
        int vidx = vpids[i];
        const cv::Vec3f& p = model->vtx(vidx);
        const cv::Vec3f dv = p - p0;
        const double h = cv::norm(dv);
        const double theta = acos( double(dv.dot(u))/h);
        const double o = h*sin(theta);
        if ( o >= maxd)
        {
            maxd = o;
            bv = vidx;
        }   // end if
    }   // end for

    if ( dout)
        *dout = maxd;

    return model->vtx(bv);
}   // end findDeepestPoint
