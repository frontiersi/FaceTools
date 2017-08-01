#ifndef FACE_TOOLS_SURFACE_MESHER_H
#define FACE_TOOLS_SURFACE_MESHER_H

#include "FaceTools_Export.h"
#include <ObjModel.h>   // RFeatures

namespace FaceTools
{

class FaceTools_EXPORT SurfaceMesher
{
public:
    explicit SurfaceMesher( RFeatures::ObjModel::Ptr);
    RFeatures::ObjModel::Ptr getObject() const { return _model;}

    // Remesh the object returning the number of triangles added.
    // Not a valid operation if the object already has triangles.
    int operator()();   // Remesh the object

private:
    RFeatures::ObjModel::Ptr _model;
};  // end class

}   // end namespace

#endif
