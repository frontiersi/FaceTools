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

#ifndef FACE_TOOLS_U3D_CACHE_H
#define FACE_TOOLS_U3D_CACHE_H

#include "FaceTypes.h"
#include <FaceViewSet.h>
#include <QTemporaryDir>
#include <QReadWriteLock>
#include <QThread>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT U3DCache : public QThread
{ Q_OBJECT
public:
    // Returns the u3dfile for the given model if present or empty string if not.
    // On return, a lock is set that is only released after caller calls release.
    // If an empty string is returned from lock, there is no need to call release.
    static QString lock( const FM*);
    static void release();

    // Locks for write the export of the model to U3D and unlocks on return.
    // If media9 is true, coordinates are transformed as (a,b,c) --> (a,-c,b)
    // to allow for LaTeX media9 inclusion of 3D figures.
    static void refresh( const FM*, bool media9=false);

    static void purge( const FM*);

protected:
    void run() override;

private:
    static std::shared_ptr<U3DCache> _singleton;
    static QReadWriteLock _lock;
    static QTemporaryDir _tmpdir;
    static FMS _cache;
    static QThread _wthread;
    static FM* _fm;
    static bool _media9;

    static std::shared_ptr<U3DCache> get();
    void _purge( const FM*);

    U3DCache(){}
    U3DCache( const U3DCache&) = delete;
    void operator=( const U3DCache&) = delete;

    static QString u3dfile( const FM*);
};  // end class

}}   // end namespace

#endif
