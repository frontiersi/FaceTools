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

#ifndef FACE_TOOLS_FACE_MODEL_CURVATURE_H
#define FACE_TOOLS_FACE_MODEL_CURVATURE_H

#include "FaceTypes.h"
#include <r3d/Curvature.h>
#include <vtkFloatArray.h>

namespace FaceTools {

class FaceTools_EXPORT FaceModelCurvature
{
public:
    using Ptr = std::shared_ptr<FaceModelCurvature>;

    static Ptr create( const r3d::Mesh&);

    const r3d::Curvature &vals() const { return *_cmap;}

    r3d::Curvature &vals() { return *_cmap;}

    // Call this if changes made to the curvature map.
    void updateArrays();

    // Vertex correspondence arrays
    vtkSmartPointer<vtkFloatArray> meanArray() const { return _mcrv;}   // Scalars
    vtkSmartPointer<vtkFloatArray> absArray() const { return _acrv;}    // Scalars
    vtkSmartPointer<vtkFloatArray> d2Array() const { return _dcrv;}     // Scalars
    vtkSmartPointer<vtkFloatArray> normals() const { return _nrms;}     // 3-vectors

private:
    r3d::Curvature::Ptr _cmap;
    vtkSmartPointer<vtkFloatArray> _nrms;
    vtkSmartPointer<vtkFloatArray> _mcrv;
    vtkSmartPointer<vtkFloatArray> _dcrv;
    vtkSmartPointer<vtkFloatArray> _acrv;

    FaceModelCurvature( const r3d::Mesh&);
    ~FaceModelCurvature();
    FaceModelCurvature( const FaceModelCurvature&) = delete;
    void operator=( const FaceModelCurvature&) = delete;
};  // end class

}   // end namespace

#endif
