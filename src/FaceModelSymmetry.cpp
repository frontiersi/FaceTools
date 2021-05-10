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

#include <FaceTools/FaceModelSymmetry.h>
#include <FaceTools/MaskRegistration.h>
#include <FaceTools/FaceModel.h>
#include <r3dvis/SurfaceMapper.h>
#include <r3d/SurfacePointFinder.h>
#include <cassert>
using FaceTools::FaceModelSymmetry;
using FaceTools::FM;


FaceModelSymmetry::Ptr FaceModelSymmetry::create( const FM *fm)
{
    assert( fm->hasMask());
    return Ptr( new FaceModelSymmetry( fm), []( const FaceModelSymmetry *d){ delete d;});
}   // end create


FaceModelSymmetry::~FaceModelSymmetry(){}


FaceModelSymmetry::FaceModelSymmetry( const FM *fm)
{
    _makeVtxSymm( fm);
    const r3d::Mesh &mesh = fm->mesh();
    using VSM = r3dvis::VertexSurfaceMapper;
    _xarr = VSM( [this]( int i, size_t){ return _vtxSymm.at(i)[0];}, 1).makeArray( mesh, "FaceModelSymmetry_X");
    _yarr = VSM( [this]( int i, size_t){ return _vtxSymm.at(i)[1];}, 1).makeArray( mesh, "FaceModelSymmetry_Y");
    _zarr = VSM( [this]( int i, size_t){ return _vtxSymm.at(i)[2];}, 1).makeArray( mesh, "FaceModelSymmetry_Z");
    _allarr = VSM( [this]( int i, size_t){ return _vtxSymm.at(i)[3];}, 1).makeArray( mesh, "FaceModelSymmetry_All");
}   // end ctor


void FaceModelSymmetry::_makeVtxSymm( const FM *fm)
{
    const r3d::Mesh &mesh = fm->mesh();
    const r3d::Mesh &mask = fm->mask();
    const r3d::KDTree &mkdt = fm->maskKDTree();

    const r3d::SurfacePointFinder maskPointFinder( mask);
    const std::unordered_map<int,int>& maskOppVtxs = MaskRegistration::maskData()->oppVtxs;

    const Mat4f T = fm->transformMatrix();
    Vec3f u = T.block<3,1>(0,0);
    Vec3f m = T.block<3,1>(0,3);

    for ( int vidx : mesh.vtxIds())
    {
        const Vec3f &p = mesh.vtx(vidx);    // Original vertex on the model

        // Find pm as the position on the mask that vertex p is closest to and mt as the triangle it's within:
        Vec3f pm;
        int mt = -1;
        int pvidx = mkdt.find( p);
        maskPointFinder.find( p, pvidx, mt, pm);
        Vec3f qm;

        if ( mt < 0)
        {
            assert( pvidx >= 0);
            assert( pm == mask.vtx(pvidx));
            qm = mask.vtx(maskOppVtxs.at(pvidx));
        }   // end if
        else
        {
            assert( pvidx == -1);
            // Find bm as the barycentric coordinates of pm in triangle mt:
            const Vec3f bm = mask.toBarycentric( mt, pm);
            // Need to manually obtain the new coordinates because the order of the vertices
            // in the opposite polygon will not match due to the surface being reflected, but
            // the normal still pointing out from the face.
            const int *fvidxs = mask.fvidxs(mt);
            assert(fvidxs);
            assert( maskOppVtxs.count(fvidxs[0]) > 0);
            assert( maskOppVtxs.count(fvidxs[1]) > 0);
            assert( maskOppVtxs.count(fvidxs[2]) > 0);
            const int v0 = maskOppVtxs.at(fvidxs[0]);
            const int v1 = maskOppVtxs.at(fvidxs[1]);
            const int v2 = maskOppVtxs.at(fvidxs[2]);
            qm = bm[0]*mask.vtx(v0) + bm[1]*mask.vtx(v1) + bm[2]*mask.vtx(v2);
        }   // end else

        // Find pr as original point p reflected through the medial plane to its perfectly symmetric position:
        const Vec3f pmr = pm + 2*(m-pm).dot(u)*u;

        const Vec3f pm2qm = qm - pm;
        const Vec3f pmr2qm = qm - pmr;
        const Vec3f pm2pmr = pmr - pm;

        Vec4f &vals = _vtxSymm[vidx];
        vals[0] = fabsf(pm2pmr[0]) - fabsf(pm2qm[0]);    // Asymmetry through medial plane (along X-axis)
        vals[1] = -pmr2qm[1];  // Asymmetry along Y-axis
        vals[2] = -pmr2qm[2];  // Asymmetry along Z-axis

        // Get the sign of the difference by comparing the distance of qm from pm with the distance of pmr from pm.
        // If (qm - pm) is greater, that means that the original masked mapped point pm with respect to its
        // anthropometrically mapped partner is closer in than expected.
        const float sgn = pm2qm.squaredNorm() >= pm2pmr.squaredNorm() ? -1 : 1;
        // Find the magnitude of difference of the anthropometrically mapped symmetric point (qm)
        // with the expected perfectly laterally symmetric point pmr and multiply this by the sign above.
        vals[3] = sgn * pmr2qm.norm();    // Signed disparity of surface to reflected point
    }   // end for
}   // end _makeVtxSymm
