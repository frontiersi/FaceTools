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

#ifndef FACE_TOOLS_FACE_MODEL_DELTA_STORE_H
#define FACE_TOOLS_FACE_MODEL_DELTA_STORE_H

#include "FaceModelDelta.h"
#include <QReadWriteLock>

namespace FaceTools {

using FMD = FaceModelDelta;

class FaceTools_EXPORT FaceModelDeltaStore
{
public:
    // Read lock is held while returned shared ptr is alive.
    static std::shared_ptr<const FMD> vals( const FM *tgt, const FM *src);

    // Set the differences on tgt to be from src.
    static void add( const FM *tgt, const FM *src);

    static bool has( const FM *tgt, const FM *src);

    // Purge associated deltas for the given model.
    static void purge( const FM*);

private:
    static std::unordered_map<const FM*, std::unordered_map<const FM*, FMD::Ptr> > _t2s;
    static std::unordered_map<const FM*, std::unordered_map<const FM*, FMD::Ptr> > _s2t;
    static QReadWriteLock _lock;
    static bool _has( const FM *tgt, const FM *src);
};  // end class

}   // end namespace

#endif
