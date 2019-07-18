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
#include <QTemporaryDir>
#include <QReadWriteLock>

namespace FaceTools {

class FaceTools_EXPORT U3DCache
{
public:
    using Filepath = std::shared_ptr<QString>;

    // Returns the u3dfile for the given model if present or empty string if not found.
    // While a filepath is held, no write updates can occur.
    static Filepath u3dfilepath( const FM*);

    // Returns true iff U3D model export is possible.
    static bool isAvailable();

    // Locks for write the export of the model to U3D and unlocks on return.
    // If media9 is true, coordinates are transformed as (a,b,c) --> (a,-c,b)
    // to allow for LaTeX media9 inclusion of 3D figures.
    // *** THIS FUNCTION calls FM::lockForRead on the passed in FaceModel ***
    static void refresh( const FM*, bool media9=false);

    static void purge( const FM*);

private:
    static QTemporaryDir _tmpdir;
    static QReadWriteLock _cacheLock;
    static std::unordered_set<const FM*> _cache;

    void _purge( const FM*);

    U3DCache(){}
    U3DCache( const U3DCache&) = delete;
    void operator=( const U3DCache&) = delete;

    static QString makeFilePath( const FM*);
};  // end class

}   // end namespace

#endif
