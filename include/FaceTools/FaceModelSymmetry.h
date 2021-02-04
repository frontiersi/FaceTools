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

#ifndef FACE_TOOLS_FACE_MODEL_SYMMETRY_H
#define FACE_TOOLS_FACE_MODEL_SYMMETRY_H

#include "FaceTypes.h"
#include <vtkFloatArray.h>

namespace FaceTools {

class FaceTools_EXPORT FaceModelSymmetry
{
public:
    using Ptr = std::shared_ptr<FaceModelSymmetry>;

    static Ptr create( const FM*);

    vtkSmartPointer<vtkFloatArray> allArray() const { return _allarr;}
    vtkSmartPointer<vtkFloatArray> xArray() const { return _xarr;}
    vtkSmartPointer<vtkFloatArray> yArray() const { return _yarr;}
    vtkSmartPointer<vtkFloatArray> zArray() const { return _zarr;}

private:
    std::unordered_map<int, Vec4f> _vtxSymm;
    vtkSmartPointer<vtkFloatArray> _allarr;
    vtkSmartPointer<vtkFloatArray> _xarr;
    vtkSmartPointer<vtkFloatArray> _yarr;
    vtkSmartPointer<vtkFloatArray> _zarr;

    void _makeVtxSymm( const FM*);
    explicit FaceModelSymmetry( const FM*);
    ~FaceModelSymmetry();
    FaceModelSymmetry( const FaceModelSymmetry&) = delete;
    void operator=( const FaceModelSymmetry&) = delete;
};  // end class

}   // end namespace

#endif
