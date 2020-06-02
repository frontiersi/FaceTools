/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_FACE_MODEL_SYMMETRY_H
#define FACE_TOOLS_FACE_MODEL_SYMMETRY_H

#include "FaceTypes.h"
#include <QReadWriteLock>

namespace FaceTools {

class FaceTools_EXPORT FaceModelSymmetry
{
public:
    // Per vertex symmetry stored for the mesh vertices.
    // Asymmetry values are the signed X, Y, Z, and signed L2 norm.
    using VtxAsymmMap = std::unordered_map<int, Vec4f>;
    using RPtr = std::shared_ptr<const VtxAsymmMap>;

    static RPtr vals( const FM*); // Read lock is held while returned shared ptr is alive.

    static void add( const FM*);

    static void purge( const FM*);

private:
    static std::unordered_map<const FM*, VtxAsymmMap> _vtxSymm;
    static QReadWriteLock _lock;
};  // end class

}   // end namespace

#endif
