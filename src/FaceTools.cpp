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
#include <OffscreenModelViewer.h>   // RVTK
#include <FaceShapeLandmarks2DDetector.h>
#include <FaceModelViewer.h>
#include <LandmarksManager.h>
#include <FaceModel.h>
#include <Transformer.h>        // RFeatures
#include <algorithm>
#include <FaceView.h>
#include <MetricCalculatorManager.h>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
using namespace RFeatures;
using namespace FaceTools::Landmark;
using namespace QtCharts;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MetricValue;


size_t FaceTools::findCommonLandmarks( std::vector<int>& lmks, const FMS& fms)
{
    IntIntMap lmkCounts;  // Collect all landmarks over all models
    for ( const FM* fm : fms)
    {
        fm->lockForRead();
        for ( int id : fm->landmarks()->ids())
            lmkCounts[id] += 1;
        fm->unlock();
    }   // end for

    // Keep only those landmarks that every model has.
    lmks.clear();
    const int n = int(fms.size());
    for ( const auto& p : lmkCounts)
    {
        if ( p.second == n)
            lmks.push_back(p.first);
    }   // end for

    return lmks.size();
}   // end findCommonLandmarks


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


namespace {

cv::Vec3f calcMeanNormalBetweenPoints( const ObjModel* model, int v0, int v1)
{
    RFeatures::DijkstraShortestPathFinder pfinder( model);
    pfinder.setEndPointVertexIndices( v0, v1);
    std::vector<int> vidxs;
    pfinder.findShortestPath( vidxs);
    const int n = int(vidxs.size()) - 1;
    cv::Vec3f nrm(0,0,0);
    for ( int i = 0; i < n; ++i)
    {
        const IntSet& sfids = model->getSharedFaces( vidxs[size_t(i)], vidxs[size_t(i+1)]);
        std::for_each( std::begin(sfids), std::end(sfids), [&](int fid){ nrm += model->calcFaceNorm(fid);});
    }   // end for
    cv::normalize( nrm, nrm);
    return nrm;
}   // end calcMeanNormalBetweenPoints


void updateNormal( const RFeatures::ObjModelKDTree* kdt, const cv::Vec3f& v0, int e0, const cv::Vec3f& v1, int e1, cv::Vec3f& nvec)
{
    const ObjModel* model = kdt->model();

    // Estimate "down" vector from cross product of base vector with current (inaccurate) face normal.
    const cv::Vec3f evec = v1 - v0;
    cv::Vec3f dvec;
    cv::normalize( evec.cross(nvec), dvec);

    // Find reference locations further down the face from e0 and e1
    const float pdelta = float(1.0 * cv::norm(evec));
    const int r0 = kdt->find( v0 + dvec * pdelta);
    const int r1 = kdt->find( v1 + dvec * pdelta);

    // The final view vector is defined as the mean normal along the path over
    // the model between the provided points and the shifted points.
    const cv::Vec3f vv0 = calcMeanNormalBetweenPoints( model, r0, e0);
    const cv::Vec3f vv1 = calcMeanNormalBetweenPoints( model, r1, e1);
    cv::normalize( vv0 + vv1, nvec);
}   // end updateNormal

}   // end namespace


// public
void FaceTools::findNormal( const RFeatures::ObjModelKDTree* kdt, const cv::Vec3f& v0, const cv::Vec3f& v1, cv::Vec3f& nvec)
{
    const int e0 = kdt->find( v0);
    const int e1 = kdt->find( v1);
    static const double MIN_DELTA = 1e-8;
    static const int MAX_TRIES = 12;

    double delta = MIN_DELTA + 1;
    int tries = 0;
    while ( fabs(delta) > MIN_DELTA && tries < MAX_TRIES)
    {
        const cv::Vec3f invec = nvec;
        updateNormal( kdt, v0, e0, v1, e1, nvec);
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


ObjModel::Ptr FaceTools::createFromSubset( const ObjModel* smod, const IntSet& vidxs)
{
    assert(smod);
    ObjModel::Ptr omod = ObjModel::create( smod->spatialPrecision());
    std::for_each( std::begin(vidxs), std::end(vidxs), [&](int vidx){ omod->addVertex( smod->vtx(vidx));});
    return omod;
}   // end createFromSubset


ObjModel::Ptr FaceTools::createFromTransformedSubset( const ObjModel* smod, const IntSet& vidxs, const cv::Matx44d& T,
                                                      std::unordered_map<int,int>* newVidxsToSource)
{
    assert(smod);
    const RFeatures::Transformer transformer(T);
    ObjModel::Ptr omod = ObjModel::create( smod->spatialPrecision());
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
    ObjModel::Ptr fmod = ObjModel::create( m->spatialPrecision());
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


cv::Vec3f FaceTools::toTarget( const ObjModelKDTree* kdt, const cv::Vec3f& s, const cv::Vec3f& t)
{
    const RFeatures::ObjModelSurfacePointFinder spfinder( kdt->model());
    int vidx = kdt->find(s);
    int notused;
    cv::Vec3f fv;
    spfinder.find( t, vidx, notused, fv);
    return fv;
}   // end toTarget


bool FaceTools::findPath( const ObjModelKDTree* kdt, const cv::Vec3f& p0, const cv::Vec3f& p1, std::list<cv::Vec3f>& pts)
{
    using namespace RFeatures;

    const ObjModel *model = kdt->model();
    const ObjModelSurfacePointFinder spfinder( model);
    int v0i = kdt->find(p0);
    int v1i = kdt->find(p1);
    int sT, fT;
    cv::Vec3f v0 = p0;
    cv::Vec3f v1 = p1;
    spfinder.find( p0, v0i, sT, v0);
    spfinder.find( p1, v1i, fT, v1);

    pts.clear();
    SurfaceCurveFinder scfinder( model);
    bool foundPath = scfinder.findPath( v0, sT, v1, fT, pts);
    if ( !foundPath)
    {
        pts.clear();
        foundPath = scfinder.findPath( v1, fT, v0, sT, pts);
        pts.reverse();
    }   // end if

    return foundPath;
}   // end findPath


cv::Vec3f FaceTools::findDeepestPoint2( const ObjModelKDTree* kdt, const cv::Vec3f& p0, const cv::Vec3f& p1, double *dout)
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
}   // end findDeepestPoint2


cv::Vec3f FaceTools::findDeepestPoint( const ObjModelKDTree* kdt, const cv::Vec3f& p0, const cv::Vec3f& p1, double *dout)
{
    std::list<cv::Vec3f> pts;
    const bool foundPath = findPath( kdt, p0, p1, pts);
    if ( !foundPath)
    {
        std::cerr << "[WARNING] FaceTools::findDeepestPoint: Failed to find path using SurfaceCurveFinder - using DijkstraShortestPathFinder instead." << std::endl;
        return findDeepestPoint2(kdt, p0, p1, dout);
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
    omv.setModel( fm->info()->cmodel());
    cv::Vec3f cpos = (d * fm->orientation().nvec()) + fm->centre();
    CameraParams cam( cpos, fm->centre(), fm->orientation().uvec(), 30);
    omv.setCamera( cam);
    return omv.snapshot();
}   // end makeThumbnail


namespace {

QtCharts::QScatterSeries* createMetricPoint( double age, double val, const QString& title, const QColor& c)
{
    QScatterSeries *dpoints = new QScatterSeries;
    dpoints->setName( title);
    dpoints->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    dpoints->setMarkerSize(10);
    dpoints->setColor(c);
    dpoints->append( age, val);
    return dpoints;
}   // end createMetricPoint

}   // end namespace


QChart* FaceTools::createChart( const QString& ethn, int8_t sex, size_t d, int mid, const FM* fm, bool withTitle)
{
    MC::Ptr mc = MCM::metric(mid);
    assert(mc);
    const GrowthData* gd = mc->growthData( ethn, sex);
    if ( !gd)
        return nullptr;

    QChart *chart = new QChart;

    double xmin = 0;
    double xmax = 0;
    mc->addSeriesToChart( chart, gd, &xmin, &xmax);

    // Add the subject data point(s)
    if ( fm)
    {
        const double age = fm->age();
        xmin = std::min<double>(xmin, age);
        xmax = std::max<double>(xmax, age);

        if ( mc->isBilateral())
        {
            const MetricValue *mvl = fm->cmetricsL().get( mid);
            const MetricValue *mvr = fm->cmetricsR().get( mid);

            if ( mvl)
                chart->addSeries( createMetricPoint( age, mvl->value(d), "Left", Qt::blue));

            if ( mvl && mvr) // Create a mean data point
            {
                double val = 0.5 * (mvl->value(d) + mvr->value(d));
                chart->addSeries( createMetricPoint( age, val, "Mean", Qt::red));
            }   // end if

            if ( mvr)
                chart->addSeries( createMetricPoint( age, mvr->value(d), "Right", Qt::darkGreen));
        }   // end if
        else
        {
            const MetricValue *mv = fm->cmetrics().get( mid);
            if ( mv)
                chart->addSeries( createMetricPoint( age, mv->value(d), "Subject", Qt::red));
        }   // end else
    }   // end if

    chart->createDefaultAxes();

    QValueAxis* xaxis = new QValueAxis;
    xaxis->setLabelFormat( "%d");

    xaxis->setRange( xmin, xmax);
    int nyears = (int(xmax) - int(xmin));   // TODO make xtick count change on resizing of window
    int tc = nyears + 1;
    if ( nyears > 16)
        tc /= 2;
    xaxis->setTickCount( tc);

    chart->setAxisX( xaxis);

    chart->legend()->setAlignment(Qt::AlignRight);
    chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    chart->setBackgroundVisible(false);
    chart->setDropShadowEnabled(false);
    chart->axisX()->setTitleText( "Age from birth (years)");
    chart->axisY()->setTitleText( QString("Distance (%1)").arg(FM::LENGTH_UNITS));

    if ( withTitle)
    {
        QString title = mc->name();
        if ( mc->dims() > 1)    // Specify which dimension of the metric is being shown
            title += QString( " (Dimension %1)").arg(d+1);

        QString demog;
        if ( !gd->ethnicity().isEmpty())
            demog = " (" + gd->ethnicity() + "; ";
        demog += toLongSexString( static_cast<Sex>(gd->sex())) + ")";

        QString src = "<em>" + gd->source();
        if ( !gd->note().isEmpty())
            src += "<br>" + gd->note();
        if ( gd->n() > 0)
            src += QString("; N=%1").arg(gd->n());
        src += "</em>";

        chart->setTitle( QString("<center><big><b>%1</b>%2</big><br>%3</center>").arg( title, demog, src));
    }   // end if

    return chart;
}   // end createChart
