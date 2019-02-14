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

#ifndef FACE_TOOLS_SURFACE_DATA_H
#define FACE_TOOLS_SURFACE_DATA_H

/**
 * A globally accessible static instance that manages computations
 * and access to FaceModel surface data records.
 */

#include "FaceTypes.h"
#include <ObjModelCurvatureMetrics.h>   // RFeatures
#include <QObject>
#include <QThread>
#include <QReadWriteLock>
#include <memory>

namespace FaceTools {

struct FaceTools_EXPORT SurfaceData
{
    using RPtr = std::shared_ptr<const SurfaceData>;
    using WPtr = std::shared_ptr<SurfaceData>;

    RFeatures::ObjModelCurvatureMap::Ptr curvature;
    RFeatures::ObjModelCurvatureMetrics *metrics;
    RFeatures::ObjModelPolygonAreas pareas;
    RFeatures::ObjModelNormals normals;
};  // end struct


// Internally used by FaceModelSurfaceData
class FaceTools_EXPORT SurfaceDataWorker : public QThread
{ Q_OBJECT
public:
    explicit SurfaceDataWorker( FM*);
    ~SurfaceDataWorker() override;

    SurfaceData::RPtr readLock();
    SurfaceData::WPtr writeLock();
    FM *fmodel;
    SurfaceData *surfaceData;
    bool working;
    QReadWriteLock lock;

signals:
    void onCalculated( const FM*);

protected:
    void run();

private:
    void reset();
};  // end class

}   // end namespace

#endif
