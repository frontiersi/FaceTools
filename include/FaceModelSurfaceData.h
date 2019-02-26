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

#ifndef FACE_TOOLS_FACE_MODEL_SURFACE_DATA_H
#define FACE_TOOLS_FACE_MODEL_SURFACE_DATA_H

/**
 * A globally accessible static instance that manages computations
 * and access to FaceModel surface data records.
 */

#include "SurfaceData.h"

namespace FaceTools {

class FaceTools_EXPORT FaceModelSurfaceData : public QObject
{ Q_OBJECT
public:
    static FaceModelSurfaceData* get();   // Get the singleton instance.

    // Returns the SurfaceData for the given FM or writing - encapsulates a read lock.
    static SurfaceData::RPtr rdata( const FM*);

    // Returns true iff computed data are available for the given model
    // and an exclusive write lock over the data may be taken. Returns false
    // if the data are not available or if the background calculation thread
    // is yet to finish (and lock has not been released).
    static bool isAvailable( const FM* fm);

    // Calculate surface data for the given model. If the data aren't present, a new
    // calculation thread is created and started in the background. Signal onCalculated
    // is fired when calculation completes. Does not block.
    static void calculate( FM*);

    // Remove all data associated with the given FM.
    // Returns true on success or if data don't exist for the model.
    // Returns false if waiting for the background processing thread to
    // complete timed out and purge failed.
    static bool purge( const FM*, unsigned long waitMsecs=ULONG_MAX);

    // Returns the SurfaceData for writing - encapsulates a write lock.
    static SurfaceData::WPtr wdata( const FM*);

signals:
    void onCalculated( const FM*);   // Fires when surface data available for the given model.

private:
    std::unordered_map<const FM*, SurfaceDataWorker*> _data;
    static std::unordered_map<const FM*, SurfaceDataWorker*>& data();
    static std::shared_ptr<FaceModelSurfaceData> s_ptr;
    FaceModelSurfaceData(){}
    ~FaceModelSurfaceData() override;
};  // end class

}   // end namespace

#endif
