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

#ifndef FACE_TOOLS_FACE_MODEL_SYMMETRY_STORE_H
#define FACE_TOOLS_FACE_MODEL_SYMMETRY_STORE_H

#include "FaceModelSymmetry.h"
#include <QReadWriteLock>

namespace FaceTools {

class FaceTools_EXPORT FaceModelSymmetryStore
{
public:
    // Read lock is held while returned shared ptr is alive.
    static std::shared_ptr<const FaceModelSymmetry> vals( const FM*);
    static bool isMapped( const FM*);
    static void add( const FM*);
    static void purge( const FM*);

private:
    static std::unordered_map<const FM*, FaceModelSymmetry::Ptr> _vtxSymm;
    static QReadWriteLock _lock;
};  // end class

}   // end namespace

#endif
