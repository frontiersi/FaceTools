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

#include <FaceTools.h>
#include <Detect/FaceShapeLandmarks2DDetector.h>
#include <Metric/MetricCalculatorManager.h>
#include <LndMrk/LandmarksManager.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Ethnicities.h>
#include <OffscreenModelViewer.h>   // RVTK
#include <Transformer.h>        // RFeatures
#include <algorithm>
using namespace RFeatures;
using namespace FaceTools::Landmark;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MetricValue;
using FaceTools::FM;


namespace {

cv::Vec3f calcMeanNormalBetweenPoints( const ObjModel& model, int v0, int v1)
{
    RFeatures::DijkstraShortestPathFinder pfinder( model);
    pfinder.setEndPointVertexIndices( v0, v1);
    std::vector<int> vidxs;
    pfinder.findShortestPath( vidxs);
    const int n = int(vidxs.size()) - 1;
    cv::Vec3f nrm(0,0,0);
    for ( int i = 0; i < n; ++i)
    {
        const IntSet& sfs = model.spolys( vidxs[size_t(i)], vidxs[size_t(i+1)]);
        std::for_each( std::begin(sfs), std::end(sfs), [&](int f){ nrm += model.calcFaceNorm(f);});
    }   // end for
    cv::normalize( nrm, nrm);
    return nrm;
}   // end calcMeanNormalBetweenPoints


void updateNormal( const FM* fm, const cv::Vec3f& v0, int e0, const cv::Vec3f& v1, int e1, cv::Vec3f& nvec)
{
    // Estimate "down" vector from cross product of base vector with current (inaccurate) face normal.
    const cv::Vec3f evec = v1 - v0;
    cv::Vec3f dvec;
    cv::normalize( evec.cross(nvec), dvec);

    // Find reference locations further down the face from e0 and e1
    const float pdelta = float(1.0 * cv::norm(evec));
    const int r0 = fm->findVertex( v0 + dvec * pdelta);
    const int r1 = fm->findVertex( v1 + dvec * pdelta);

    // The final view vector is defined as the mean normal along the path over
    // the model between the provided points and the shifted points.
    const cv::Vec3f vv0 = calcMeanNormalBetweenPoints( fm->model(), r0, e0);
    const cv::Vec3f vv1 = calcMeanNormalBetweenPoints( fm->model(), r1, e1);
    cv::normalize( vv0 + vv1, nvec);
}   // end updateNormal

}   // end namespace


// public
void FaceTools::findNormal( const FM* fm, const cv::Vec3f& v0, const cv::Vec3f& v1, cv::Vec3f& nvec)
{
    const int e0 = fm->findVertex( v0);
    const int e1 = fm->findVertex( v1);
    static const double MIN_DELTA = 1e-8;
    static const int MAX_TRIES = 12;

    double delta = MIN_DELTA + 1;
    int tries = 0;
    while ( fabs(delta) > MIN_DELTA && tries < MAX_TRIES)
    {
        const cv::Vec3f invec = nvec;
        updateNormal( fm, v0, e0, v1, e1, nvec);
        delta = cv::norm( nvec - invec);
        tries++;
    }   // end while
}   // end findNormal


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


ObjModel::Ptr FaceTools::createFromSubset( const ObjModel& smod, const IntSet& vidxs)
{
    ObjModel::Ptr omod = ObjModel::create();
    std::for_each( std::begin(vidxs), std::end(vidxs), [&](int vidx){ omod->addVertex( smod.vtx(vidx));});
    return omod;
}   // end createFromSubset


ObjModel::Ptr FaceTools::createFromTransformedSubset( const ObjModel& smod, const IntSet& vidxs, const cv::Matx44d& T,
                                                      std::unordered_map<int,int>* newVidxsToSource)
{
    const RFeatures::Transformer transformer(T);
    ObjModel::Ptr omod = ObjModel::create();
    for ( int vidx : vidxs)
    {
        cv::Vec3f v = smod.vtx(vidx);
        transformer.transform( v);
        const int nvidx = omod->addVertex(v);
        if ( newVidxsToSource)
            (*newVidxsToSource)[nvidx] = vidx;
    }   // end for
    return omod;
}   // end createFromTransformedSubset


// Flatten m to XY plane and return it, also setting fmap to be the
// vertex ID mapping from the returned flattened object to the original object m.
ObjModel::Ptr FaceTools::makeFlattened( const ObjModel& m, std::unordered_map<int,int>* fmap)
{
    if ( fmap)
        fmap->clear();
    int nvidx;
    ObjModel::Ptr fmod = ObjModel::create();
    const IntSet& vids = m.vtxIds();
    for ( int vidx : vids)
    {
        cv::Vec3f v = m.vtx(vidx);
        v[2] = 0;
        nvidx = fmod->addVertex(v);
        if ( fmap)
            (*fmap)[nvidx] = vidx;
    }   // end for
    // fmod is now a flattened version
    return fmod;
}   // end makeFlattened


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


cv::Vec3f FaceTools::toSurface( const FM* fm, const cv::Vec3f& v)
{
    int notused;
    cv::Vec3f fv;
    const RFeatures::ObjModelSurfacePointFinder spfinder( fm->model());
    int vidx = fm->findVertex(v);
    spfinder.find( v, vidx, notused, fv);
    return fv;
}   // end toSurface


cv::Vec3f FaceTools::toTarget( const FM* fm, const cv::Vec3f& s, const cv::Vec3f& t)
{
    const RFeatures::ObjModelSurfacePointFinder spfinder( fm->model());
    int vidx = fm->findVertex(s);
    int notused;
    cv::Vec3f fv;
    spfinder.find( t, vidx, notused, fv);
    return fv;
}   // end toTarget


bool FaceTools::findPath( const FM* fm, const cv::Vec3f& p0, const cv::Vec3f& p1, std::list<cv::Vec3f>& pts)
{
    RFeatures::ObjModelSurfaceCurveFinder scfinder0( fm->model(), fm->kdtree());
    RFeatures::ObjModelSurfaceCurveFinder scfinderR( fm->model(), fm->kdtree());
 
    //std::cerr << "Finding path with endpoints: " << p0 << ", " << p1;
    double psum0 = scfinder0.findPath( p0, p1);
    double psumr = scfinderR.findPath( p1, p0);
    if ( psum0 + psumr < 0.0)
    {
        //std::cerr << " not found!" << std::endl;
        return false;
    }   // end if

    if ( psum0 < 0)
        psum0 = DBL_MAX;
    if ( psumr < 0)
        psumr = DBL_MAX;

    const std::vector<cv::Vec3f>* lpath = &scfinder0.lastPath();
    if ( psumr < psum0)
        lpath = &scfinderR.lastPath();
    assert( lpath);
    pts = std::list<cv::Vec3f>( lpath->begin(), lpath->end());
    if ( psumr < psum0)
        pts.reverse();

    //std::cerr << " surface path length = " << std::min(psumr, psum0) << std::endl;
    return true;
}   // end findPath


bool FaceTools::findStraightPath( const FM* fm, const cv::Vec3f& p0, const cv::Vec3f& p1, const cv::Vec3f& focVec, std::list<cv::Vec3f>& pts)
{
    ObjModelSurfaceGlobalPlanePathFinder pfinder( fm->model(), fm->kdtree(), focVec);
    if ( pfinder.findPath( p0, p1) >= 0)
    {
        const std::vector<cv::Vec3f>& lpath = pfinder.lastPath();
        pts = std::list<cv::Vec3f>( lpath.begin(), lpath.end());
    }   // end if
    return !pts.empty();
}   // end findStraightPath


bool FaceTools::findCurveFollowingPath( const FM* fm, const cv::Vec3f& p0, const cv::Vec3f& p1, std::list<cv::Vec3f>& pts)
{
    ObjModelSurfaceLocalPlanePathFinder pfinder( fm->model(), fm->kdtree());
    if ( pfinder.findPath( p0, p1) >= 0)
    {
        const std::vector<cv::Vec3f>& lpath = pfinder.lastPath();
        pts = std::list<cv::Vec3f>( lpath.begin(), lpath.end());
    }   // end if
    return !pts.empty();
}   // end findCurveFollowingPath


cv::Vec3f FaceTools::findDeepestPoint2( const FM* fm, const cv::Vec3f& p0, const cv::Vec3f& p1, double *dout)
{
    const ObjModel& model = fm->model();
    DijkstraShortestPathFinder spfinder( model);
    const int v0 = fm->findVertex(p0);
    const int v1 = fm->findVertex(p1);
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
        const cv::Vec3f& p = model.vtx(vidx);
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

    return model.vtx(bv);
}   // end findDeepestPoint2


cv::Vec3f FaceTools::findDeepestPoint( const FM* fm, const cv::Vec3f& p0, const cv::Vec3f& p1, double *dout)
{
    std::list<cv::Vec3f> pts;
    const bool foundPath = findPath( fm, p0, p1, pts);
    if ( !foundPath)
    {
        std::cerr << "[WARNING] FaceTools::findDeepestPoint: Failed to find path using SurfaceCurveFinder - using DijkstraShortestPathFinder instead." << std::endl;
        return findDeepestPoint2( fm, p0, p1, dout);
    }   // end if

    cv::Vec3f u;
    cv::normalize( p1 - p0, u);

    double maxd = 0;
    cv::Vec3f dp;   // Deepest point
    for ( const cv::Vec3f& p : pts)
    {
        const cv::Vec3f dv = p - p0;
        const double h = cv::norm(dv);
        const double theta = acos( double(dv.dot(u))/h);
        const double o = h*sin(theta);
        if ( o >= maxd)
        {
            maxd = o;
            dp = p;
        }   // end if
    }   // end for

    if ( dout)
        *dout = maxd;

    return dp;
}   // end findDeepestPoint


cv::Mat_<cv::Vec3b> FaceTools::makeThumbnail( const FM* fm, const cv::Size& dims, float d)
{
    RVTK::OffscreenModelViewer omv( dims, 1);
    omv.setModel( fm->model());
    const cv::Vec3f centre = fm->centre();
    const RFeatures::Orientation on = fm->orientation();
    const cv::Vec3f cpos = (d * on.nvec()) + centre;
    const CameraParams cam( cpos, centre, on.uvec(), 30);
    omv.setCamera( cam);
    return omv.snapshot();
}   // end makeThumbnail


/*
QColor FaceTools::chooseContrasting( const QColor& a)
{
    QColor b;
    b.setRed( abs(a.red() - 255) > a.red() ? 255 : 0);
    b.setGreen( abs(a.green() - 255) > a.green() ? 255 : 0);
    b.setBlue( abs(a.blue() - 255) > a.blue() ? 255 : 0);
    return b;
}   // end chooseContrasting
*/


QColor FaceTools::chooseContrasting( const QColor& a)
{
    QColor b;
    double nr = 0.299*a.red();
    double ng = 0.587*a.green();
    double nb = 0.114*a.blue();
    if ( (nr + ng + nb) > 186.0)
        b = Qt::black;
    else
        b = Qt::white;
    return b;
}   // end chooseContrasting
