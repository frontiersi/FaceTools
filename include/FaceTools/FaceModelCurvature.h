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

#ifndef FACE_TOOLS_FACE_MODEL_CURVATURE_H
#define FACE_TOOLS_FACE_MODEL_CURVATURE_H

#include "FaceTypes.h"
#include <r3d/Curvature.h>
#include <QReadWriteLock>

namespace FaceTools {

class FaceTools_EXPORT FaceModelCurvature
{
public:
    using RPtr = std::shared_ptr<const r3d::Curvature>;
    using WPtr = std::shared_ptr<r3d::Curvature>;

    // Returns the curvature map for the given model or null if not available.
    // Read lock is held while returned shared ptr is alive.
    static RPtr rmetrics( const FM*);

    // Returns the curvature map for the given model or null if not available.
    // Write lock is held while returned shared ptr is alive.
    static WPtr wmetrics( const FM*);

    // Delete curvature data associated with the given model.
    static void purge( const FM*);

    // Create and add curvature data for the given model.
    static void add( const FM*);

private:
    static std::unordered_map<const FM*, r3d::Curvature::Ptr> _metrics;
    static QReadWriteLock _lock;
};  // end class

}   // end namespace

#endif
