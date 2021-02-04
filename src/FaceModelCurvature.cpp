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

#include <FaceModelCurvature.h>
#include <r3dvis/SurfaceMapper.h>
#include <r3dvis/VtkTools.h>    // makeNormals
#include <r3d/CurvatureMetrics.h>
using FaceTools::FaceModelCurvature;


FaceModelCurvature::Ptr FaceModelCurvature::create( const r3d::Mesh &mesh)
{
    return Ptr( new FaceModelCurvature(mesh), []( FaceModelCurvature* d){ delete d;});
}   // end create


// private
FaceModelCurvature::FaceModelCurvature( const r3d::Mesh &mesh)
    : _cmap( r3d::Curvature::create( mesh))
{
    updateArrays();
}   // end ctor

// private
FaceModelCurvature::~FaceModelCurvature(){}


void FaceModelCurvature::updateArrays()
{
    const r3d::Mesh &mesh = _cmap->mesh();
    _nrms = r3dvis::makeNormals( *_cmap);
    _nrms->SetName("Normals");
    const r3d::CurvatureMetrics cm( *_cmap);
    const auto meanCurvFn = [&]( int i, size_t)
            { return 90.0f * (cm.vertexKP1FirstOrder(i) + cm.vertexKP2FirstOrder(i))/2;};
    const auto absCurvFn = [&]( int i, size_t)
            { return 45.0f * (fabsf(cm.vertexKP1FirstOrder(i)) + fabsf(cm.vertexKP2FirstOrder(i)));};
    const auto d2CurvFn = [&]( int i, size_t) { return cm.vertexDeterminant(i);};
    using VSM = r3dvis::VertexSurfaceMapper;
    _mcrv = VSM( meanCurvFn, 1).makeArray( mesh, "FaceModelCurvature_Mean");
    _acrv = VSM(  absCurvFn, 1).makeArray( mesh, "FaceModelCurvature_Abs");
    _dcrv = VSM(   d2CurvFn, 1).makeArray( mesh, "FaceModelCurvature_D2");
}   // end updateArrays
