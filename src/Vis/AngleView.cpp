/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Vis/AngleView.h>
using FaceTools::Vis::AngleView;
using FaceTools::Vec3f;


AngleView::AngleView() : SimpleView()
{
    addActor(_ray0);
    addActor(_ray1);
    addActor(_arc);
    for ( vtkActor* actor : _actors)
    {
        vtkProperty *prop = actor->GetProperty();
        prop->SetRepresentationToWireframe();
        prop->SetRenderLinesAsTubes(false);
    }   // end for
    _arc->UseNormalAndAngleOn();
    _arc->SetResolution(27);    // 27 line segments to draw the arc
}   // end ctor


void AngleView::update( const Vec3f &fp1, const Vec3f &fp2, const Vec3f &fc, const Vec3f &fnrm, float degs)
{
    using Vec3d = Eigen::Vector3d;
    Vec3d c = fc.cast<double>();
    Vec3d nrm = fnrm.cast<double>();

    // Set the arc and its orientation and the start points for the rays
    _arc->SetNormal( &nrm[0]);
    _arc->SetCenter( &c[0]);
    _ray0->SetPoint1( &c[0]);
    _ray1->SetPoint1( &c[0]);

    Vec3d p1 = fp1.cast<double>();
    Vec3d p2 = fp2.cast<double>();
    _ray0->SetPoint2( &p1[0]);
    _ray1->SetPoint2( &p2[0]);

    Vec3d r1 = p1 - c;
    Vec3d r2 = p2 - c;
    _arc->SetAngle( degs);
    // If the angle is larger than 180 degrees then we need to show the larger arc
    // (by default vtkArcSource always shows the inner angle between the two rays).
    if ( nrm.dot( r1.cross(r2)) < 0.0)
        _arc->SetNegative(true);

    // The magnitude of the polar vector can only be as long as the shortest ray
    const double r1norm = r1.norm();
    const double r2norm = r2.norm();
    if ( r1norm > r2norm)
        r1 *= r2norm/r1norm;
    r1 *= 0.5;
    _arc->SetPolarVector( &r1[0]);

    _ray0->Update();
    _ray1->Update();
    _arc->Update();
}   // end update


void AngleView::setLineWidth( double lw)
{
    for ( vtkActor* actor : _actors)
    {
        vtkProperty *prop = actor->GetProperty();
        prop->SetLineWidth( lw);
    }   // end for
}   // end setLineWidth
