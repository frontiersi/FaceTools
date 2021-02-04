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

#ifndef FACE_TOOLS_FACE_MODEL_DELTA_H
#define FACE_TOOLS_FACE_MODEL_DELTA_H

#include "FaceTypes.h"
#include <vtkFloatArray.h>
#include <r3d/Mesh.h>

namespace FaceTools {

class FaceTools_EXPORT FaceModelDelta
{
public:
    using Ptr = std::shared_ptr<FaceModelDelta>;

    // Create and return the delta between the two models iff
    // the underlying masks match otherwise return null.
    static Ptr create( const FM *tgt, const FM *src);

    inline const FM *target() const { return _tgt;}
    inline const FM *source() const { return _src;}

    inline const r3d::Mesh& alignedSourceMask() const { return *_asmsk;}

    // Get the vertex scalar arrays for visualisation on the TARGET surface
    vtkSmartPointer<vtkFloatArray> perpArray() const { return _perpArr;}
    vtkSmartPointer<vtkFloatArray> angdArray() const { return _angdArr;}
    vtkSmartPointer<vtkFloatArray> smagArray() const { return _smagArr;}

    // Get the vertex vector array for visualising vectors on the SOURCE mask
    vtkSmartPointer<vtkFloatArray> vecsArray() const { return _vecsArr;}
    vtkSmartPointer<vtkFloatArray> sclsArray() const { return _sclsArr;}

private:
    const FM *_tgt;
    const FM *_src;
    r3d::Mesh::Ptr _asmsk;

    // Per vertex values
    struct VtxVals
    {
        Vec3f dvector;  // Difference of target from source
        Vec3f scalars;  // Scalar values as below:
    };  // end struct
    std::unordered_map<int, VtxVals> _maskVtxVals;  // Mask vertex differences
    std::unordered_map<int, Vec3f> _targVtxVals;  // Target vertex scalars (perp, angd, smag)

    vtkSmartPointer<vtkFloatArray> _perpArr;    // For target face
    vtkSmartPointer<vtkFloatArray> _angdArr;    // For target face
    vtkSmartPointer<vtkFloatArray> _smagArr;    // For target face
    vtkSmartPointer<vtkFloatArray> _vecsArr;    // For source mask
    vtkSmartPointer<vtkFloatArray> _sclsArr;    // For source mask

    void _calcMaskVtxVals();
    void _calcTargetMeshVtxVals();
    FaceModelDelta( const FM *tgt, const FM *src);
    ~FaceModelDelta();
    FaceModelDelta( const FaceModelDelta&) = delete;
    void operator=( const FaceModelDelta&) = delete;
};  // end class

}   // end namespace

#endif
