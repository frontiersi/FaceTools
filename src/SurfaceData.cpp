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

#include <SurfaceData.h>
#include <FaceModel.h>
#include <ObjModelTriangleMeshParser.h>     // RFeatures
#include <QApplication>
#include <cassert>
using FaceTools::SurfaceDataWorker;
using FaceTools::SurfaceData;
using FaceTools::FaceModel;

using RFeatures::ObjModelTriangleMeshParser;
using RFeatures::ObjModelCurvatureMetrics;
using RFeatures::ObjModelCurvatureMap;
using RFeatures::ObjModelPolygonAreas;
using RFeatures::ObjModelNormals;
using RFeatures::ObjModelInfo;
using RFeatures::ObjModel;

namespace {


int checkSame( const IntSet* s0, const IntSet* s1)
{
    if ( s0->size() != s1->size())
        return (int)s0->size() - (int)s1->size();

    for ( int f : *s0)
        if ( s1->count(f) == 0)
            return -1;

    return 0;
}   // end checkSame


int findMaxZPolygon( const ObjModel* model, const IntSet* vidxs)
{
    int zp = -1;
    float zval = -FLT_MAX;
    for ( int vid : *vidxs)
    {
        const float zt = model->vtx(vid)[2];
        if ( zt > zval)
        {
            zp = vid;
            zval = zt;
        }   // end if
    }   // end for
    return *model->getFaceIds(zp).begin();
}   // end findMaxZPolygon


void createSurfaceData( SurfaceData* sd, const FaceModel* fm)
{
    const ObjModelInfo::Ptr info = fm->info();
    const int nc = (int)info->components().size();   // # components

    // Parse all the components in the model to extract normals and polygon areas
    const ObjModel* model = info->cmodel();
    ObjModelTriangleMeshParser parser(model);
    parser.addTriangleParser( &sd->normals);
    parser.addTriangleParser( &sd->pareas);
    for ( int c = 0; c < nc; ++c)
    {
        const IntSet* cvidxs = info->components().componentVertices(c);
        const int zp = findMaxZPolygon( model, cvidxs);
#ifndef NDEBUG
        const IntSet* cpolys = info->components().componentPolygons(c);
        IntSet pset;    // Will be the IDs of the faces parsed for the component
        parser.setParseSet( &pset);
#endif
        parser.parse( zp, cv::Vec3d(0,0,1));
#ifndef NDEBUG
        int dval = checkSame( cpolys, &pset);
        if ( dval != 0)
        {
            std::cerr << "[ERROR] Parsed contents doesn't match known polygons of component "
                      << c << " (" << dval << ")" << std::endl;
            assert(false);
        }   // end if
#endif
    }   // end for

    // Now map the curvature to the surface of each of the components
    sd->curvature = ObjModelCurvatureMap::create( model, &sd->normals, &sd->pareas);
    for ( int c = 0; c < nc; ++c)
    {
        //std::cerr << "[INFO] FaceTools::SurfaceData::createSurfaceData: Mapping curvature to model component " << c << std::endl;
        const IntSet* cverts = info->components().componentVertices(c);
        assert(cverts);
        sd->curvature->map( *cverts);
    }   // end for
    sd->metrics = new ObjModelCurvatureMetrics( sd->curvature.get());
}   // end createSurfaceData

}   // end namespace


SurfaceDataWorker::SurfaceDataWorker( FaceModel* fm)
    : fmodel(fm), surfaceData( new SurfaceData), working(false)
{
}   // end ctor

SurfaceDataWorker::~SurfaceDataWorker()
{
    delete surfaceData->metrics;
    delete surfaceData;
}   // end dtor


void SurfaceDataWorker::calculate()
{
    working = true;
    lock.lockForWrite();
    fmodel->lockForRead();

    // Create SurfaceData via a worker thread
    QThread *wthread = QThread::create( [=](){ createSurfaceData( surfaceData, fmodel);});
    moveToThread( wthread);
    connect( wthread, &QThread::finished, [=](){
            moveToThread( QApplication::instance()->thread());
            fmodel->unlock();
            working = false;
            lock.unlock();
            emit onCalculated(fmodel);
            });
    connect( wthread, &QThread::finished, wthread, &QObject::deleteLater);
    wthread->start();
}   // end calculate


SurfaceData::RPtr SurfaceDataWorker::readLock()
{
    // Note here the requirement to lock mutex for the model itself; that's because the surface metrics directly
    // reference (hold a constant pointer to) the FaceModel's ObjModel which should not be allowed to change while
    // the metrics are being read.
    lock.lockForRead();
    fmodel->lockForRead();
    return SurfaceData::RPtr( surfaceData, [this](auto p){ this->fmodel->unlock(); this->lock.unlock();});
}   // end readLock


SurfaceData::WPtr SurfaceDataWorker::writeLock()
{
    // Similarly, if updating the surface data metrics, it is implicit that the model itself is changing
    // so a write lock is provided on it here.
    lock.lockForWrite();
    fmodel->lockForWrite();
    return SurfaceData::WPtr( surfaceData, [this](auto p){ this->fmodel->unlock(); this->lock.unlock();});
}   // end writeLock

