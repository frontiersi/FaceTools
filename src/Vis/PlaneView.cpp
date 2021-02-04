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

#include <Vis/PlaneView.h>
using FaceTools::Vis::PlaneView;
using FaceTools::Vec3f;


PlaneView::PlaneView() : SimpleView()
{
    addActor(_planeSource);
}   // end ctor


void PlaneView::update( const Vec3f &centre, const Vec3f &normal, float sz)
{
    using Vec3d = Eigen::Vector3d;
    Vec3d c = centre.cast<double>();
    Vec3d n = normal.cast<double>();

    _planeSource->SetOrigin( c[0], c[1], c[2]);
    _planeSource->SetPoint1( c[0] + sz,  c[1], c[2]);
    _planeSource->SetPoint2( c[0], c[1] + sz,  c[2]);

    _planeSource->SetCenter(&c[0]);
    _planeSource->SetNormal(&n[0]);
    _planeSource->Update();
}   // end update
