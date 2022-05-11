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

#include <FaceTools/FaceModelDelta.h>
#include <FaceTools/FaceModel.h>
#include <r3d/SurfacePointFinder.h>
#include <r3d/ProcrustesSuperimposition.h>
#include <r3dvis/SurfaceMapper.h>
#include <cassert>
using FaceTools::FaceModelDelta;
using FaceTools::FM;

namespace {
r3d::Mesh::Ptr calcAlignedSourceMask( const r3d::Mesh &tmsk, const r3d::Mesh &smsk)
{
    assert( tmsk.hasSequentialIds());
    assert( smsk.hasSequentialIds());
    const int NVTXS = int(tmsk.numVtxs());
    assert( NVTXS == int(smsk.numVtxs()));

    r3d::MatX3f tgtMaskVtxRows( NVTXS, 3);
    r3d::MatX3f srcMaskVtxRows( NVTXS, 3);
    for ( int vidx = 0; vidx < NVTXS; ++vidx)
    {
        tgtMaskVtxRows.row(vidx) = tmsk.uvtx(vidx);
        srcMaskVtxRows.row(vidx) = smsk.uvtx(vidx);
    }   // end for

    // Get the transformation matrix that best aligns the source with the target.
    // Note that the alignment of the source with the target by applying Procrustes
    // superimposition will be different than if we just align both the source and
    // the target separately to the template mask!
    const r3d::Mat4f T = r3d::ProcrustesSuperimposition( tgtMaskVtxRows, true)( srcMaskVtxRows);

    r3d::Mesh::Ptr asmsk = smsk.deepCopy();
    asmsk->addTransformMatrix(T);
    asmsk->fixTransformMatrix();
    return asmsk;
}   // end calcAlignedSourceMask
}   // end namespace


FaceModelDelta::Ptr FaceModelDelta::create( const FM *tgt, const FM *src)
{
    if ( !tgt->hasMask() || !src->hasMask() || tgt->maskHash() != src->maskHash())
    {
        std::cerr << "[ERROR] FaceModelDelta::create: Mask mismatch!" << std::endl;
        assert(false);
        return nullptr;
    }   // end if
    return Ptr( new FaceModelDelta( tgt, src), []( const FaceModelDelta *d){ delete d;});
}   // end create


FaceModelDelta::FaceModelDelta( const FM *fmt, const FM *fms) : _tgt(fmt), _src(fms)
{
    _asmsk = calcAlignedSourceMask( fmt->mask(), fms->mask());
    _calcMaskVtxVals();
    _calcTargetMeshVtxVals();
    using VSM = r3dvis::VertexSurfaceMapper;

    // Make the source mask vector array. Note that the scalars array contains negative values
    // which will flip the direction of those difference vectors so we negate those ones.
    _vecsArr = VSM( [this](int vid, size_t k)
                    {
                        const float neg = copysignf( 1.0f, _maskVtxVals.at(vid).scalars[2]);
                        return neg * _maskVtxVals.at(vid).dvector[k];
                    }, 3).makeArrayNoTx( *_asmsk, "FaceModelDelta_Vectors");

    // Make the source mask scalars array (contains negative values!)
    _sclsArr = VSM( [this](int vid, size_t)
                    { return _maskVtxVals.at(vid).scalars[2];}, 1).makeArrayNoTx( *_asmsk, "FaceModelDelta_Scalars");

    // Make the scalar arrays
    const r3d::Mesh &mesh = _tgt->mesh();
    _perpArr = VSM( [this](int vid, size_t)
                    { return _targVtxVals.at(vid)[0];}, 1).makeArray( mesh, "FaceModelDelta_Perpendicular");

    _angdArr = VSM( [this](int vid, size_t)
                    { return _targVtxVals.at(vid)[1];}, 1).makeArray( mesh, "FaceModelDelta_Angular");
    _smagArr = VSM( [this](int vid, size_t)
                    { return _targVtxVals.at(vid)[2];}, 1).makeArray( mesh, "FaceModelDelta_SignedMag");
}   // end ctor


FaceModelDelta::~FaceModelDelta() {}


void FaceModelDelta::_calcMaskVtxVals()
{
    const r3d::Mesh &tmsk = _tgt->mask();  // Original mask from the target model
    const r3d::Mesh &asmsk = *_asmsk;
    for ( int vidx : tmsk.vtxIds())
    {
        VtxVals &vvals = _maskVtxVals[vidx];
        vvals.dvector = tmsk.uvtx( vidx) - asmsk.uvtx( vidx);
        const Vec3f &dv = vvals.dvector;

        const Vec3f snrm = asmsk.calcVertexNorm( vidx);   // Normal direction on source mask for this vertex
        const float dp = dv.dot(snrm); // Signed magnitude of difference along vector perpendicular to source
        vvals.scalars[0] = dp;
        vvals.scalars[1] = (dv - dp * snrm).norm();  // Transverse difference
        vvals.scalars[2] = copysignf( 1.0f, dp) * dv.norm();   // Signed total change
    }   // end for
}   // end _calcMaskVtxVals


void FaceModelDelta::_calcTargetMeshVtxVals()
{
    const r3d::Mesh &mesh = _tgt->mesh();
    const r3d::Mesh &mask = _tgt->mask();
    const r3d::KDTree &mkdt = _tgt->maskKDTree();
    const r3d::SurfacePointFinder maskPointFinder( mask);

    for ( int vidx : mesh.vtxIds())
    {
        const Vec3f &p = mesh.uvtx(vidx);    // Original vertex on target to which we're mapping differences
        // Find pm as mask position that p is closest to and fid as the triangle it's in:
        Vec3f pm;
        int fid = -1;
        int pvidx = mkdt.find( p);
        maskPointFinder.find( p, pvidx, fid, pm);

        if ( fid < 0)    // The mesh point vidx is perfectly coincident with mask point pvidx
        {
            assert( pvidx >= 0);
            assert( pm == mask.uvtx(pvidx));
            assert( _maskVtxVals.count(pvidx) > 0);
            _targVtxVals[vidx] = _maskVtxVals.at(pvidx).scalars;
        }   // end if
        else
        {
            assert( pvidx == -1);
            // Find the barycentric values
            const Vec3f bm = mask.toBarycentric( fid, pm);
            const int *fvidxs = mask.fvidxs(fid);
            assert( _maskVtxVals.count(fvidxs[0]) > 0);
            assert( _maskVtxVals.count(fvidxs[1]) > 0);
            assert( _maskVtxVals.count(fvidxs[2]) > 0);
            _targVtxVals[vidx] = bm[0]*_maskVtxVals.at(fvidxs[0]).scalars
                               + bm[1]*_maskVtxVals.at(fvidxs[1]).scalars
                               + bm[2]*_maskVtxVals.at(fvidxs[2]).scalars;
        }   // end else
    }   // end for
}   // end _calcTargetMeshVtxVals
