/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include "FaceModel.h"
#include <r3d/Colour.h>
#include <QTemporaryDir>
#include <QReadWriteLock>

namespace FaceTools {

class FaceTools_EXPORT U3DCache
{
public:
    using Filepath = std::shared_ptr<QString>;

    // Returns the u3dfile for the given model if present or empty string if not found.
    // While a filepath is held, no write updates can occur.
    static Filepath u3dfilepath( const FM&);

    // Returns true iff U3D model export is possible.
    static bool isAvailable();

    // Locks for write the export of the model to U3D and unlocks on return.
    // Returns true iff model was updated in the cache successfully.
    static bool refresh( const FM&);

    static void purge( const FM&);

    // Makes a U3D model from the current colour visualisation on fv
    // and saves at u3dfilepath. This function blocks. Returns the
    // generated copy of the model with the scalar texture on success.
    static r3d::Mesh::Ptr makeColourMappedU3D( const Vis::FV *fv, const QString &u3dfilepath);

private:
    static QTemporaryDir _tmpdir;
    static QReadWriteLock _rwLock;
    static std::unordered_map<const FM*, QString> _cache;

    static bool _exportU3D( const r3d::Mesh&, const QString&, const r3d::Colour &ems);
    U3DCache(){}
    U3DCache( const U3DCache&) = delete;
    void operator=( const U3DCache&) = delete;
};  // end class

}   // end namespace

#endif
