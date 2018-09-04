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

#ifndef FACE_TOOLS_SURFACE_MESHER_H
#define FACE_TOOLS_SURFACE_MESHER_H

// Disable warnings about long function names being replaced.
#ifdef _WIN32
#pragma warning( disable : 4503)
#endif

/**
 * Take a point cloud object (only vertices - at least three), and generate
 * a polygonal triangulated surface mesh for it.
 */

#include "FaceTools_Export.h"
#include <ObjModel.h>   // RFeatures

namespace FaceTools {

class FaceTools_EXPORT SurfaceMesher
{
public:
    explicit SurfaceMesher( RFeatures::ObjModel::Ptr);  // Takes an object without polygons - just vertices.
    RFeatures::ObjModel::Ptr getObject() const { return _model;}

    // Remesh the object returning the number of triangles added.
    // Not a valid operation if the object already has triangles.
    int operator()();   // Remesh the object

private:
    RFeatures::ObjModel::Ptr _model;
};  // end class

}   // end namespace

#endif
