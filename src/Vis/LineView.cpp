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

#include <Vis/LineView.h>
#include <vtkProperty.h>
using FaceTools::Vis::LineView;
using FaceTools::Vec3f;


LineView::LineView() : SimpleView()
{
    vtkProperty *prop = addActor(_line)->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
}   // end ctor


void LineView::update( const Vec3f &fp1, const Vec3f &fp2)
{
    using Vec3d = Eigen::Vector3d;
    Vec3d p1 = fp1.cast<double>();
    Vec3d p2 = fp2.cast<double>();
    _line->SetPoint1( &p1[0]);
    _line->SetPoint2( &p2[0]);
    _line->Update();
}   // end update
