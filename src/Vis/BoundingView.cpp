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

#include <Vis/BoundingView.h>
#include <r3dvis/VtkActorCreator.h>
using FaceTools::Vis::BoundingView;
using r3d::Vec3f;


namespace {
std::vector<Vec3f> drawLines( const Vec3f &p, const Vec3f &q)
{
    std::vector<Vec3f> lns(8);
    lns[0] = Vec3f( q[0], p[1], p[2]);
    lns[1] = Vec3f( q[0], q[1], p[2]);
    lns[2] = Vec3f( p[0], q[1], p[2]);
    lns[3] = Vec3f( p[0], p[1], p[2]);
    lns[4] = Vec3f( q[0], p[1], p[2]);
    lns[5] = Vec3f( q[0], p[1], q[2]);
    lns[6] = Vec3f( q[0], q[1], q[2]);
    lns[7] = Vec3f( q[0], q[1], p[2]);
    return lns;
}   // end drawLines


void addDashes( std::vector<Vec3f> &dlns, const Vec3f &p, const Vec3f &q, int nDashes)
{
    const int N = 2*nDashes-1;
    const Vec3f gap = (q - p) / N;    // Gap between line segs
    // Only add odd numbered line segments
    for ( int i = 1; i <= N; i+=2)
    {
        dlns.push_back( p + (i-1)*gap);
        dlns.push_back( p + i*gap);
    }   // end for
}   // end addDashes


std::vector<Vec3f> drawDashedLines( const Vec3f &p, const Vec3f &q, int nDashes)
{
    const std::vector<Vec3f> lns = drawLines( p, q);
    const int N = int(lns.size());
    std::vector<Vec3f> dlns;
    dlns.reserve( N * nDashes);
    for ( int i = 1; i < N; ++i)
        addDashes( dlns, lns[i-1], lns[i], nDashes);
    return dlns;
}   // end drawDashedLines
}   // end namespace


// cb as xmin, xmax, ymin, ymax, zmin, zmax
void BoundingView::update( const r3d::Vec6f &cb, int nDashesPerEdge)
{
    reset();
    const Vec3f p( cb[0], cb[2], cb[4]);
    const Vec3f q( cb[1], cb[3], cb[5]);
    nDashesPerEdge = std::max( 1, nDashesPerEdge);
    _initActor( r3dvis::VtkActorCreator::generateLinePairsActor( drawDashedLines( p, q, nDashesPerEdge)));
    _initActor( r3dvis::VtkActorCreator::generateLinePairsActor( drawDashedLines( q, p, nDashesPerEdge)));
}   // end update


void BoundingView::_initActor( const vtkSmartPointer<vtkActor> &act)
{
    act->SetPickable(false);
    vtkProperty *prop = initActor( act);
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
}   // end initActor
