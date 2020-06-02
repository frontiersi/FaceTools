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

#include <Vis/AsymmetryView.h>
#include <vtkTransform.h>
using FaceTools::Vis::AsymmetryView;
using FaceTools::Vec3f;


AsymmetryView::AsymmetryView() : SimpleView()
{
    _arrow->SetTipResolution(22);
    _arrow->SetShaftResolution(22);
    _transformPD->SetInputConnection(_arrow->GetOutputPort());
    _arrowActor = addActor(_transformPD);

    _lineActor = addActor(_line);
    vtkProperty *prop = _lineActor->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
}   // end ctor


void AsymmetryView::setLineColour( double r, double g, double b, double a)
{
    setActorColour( _lineActor, r, g, b, a);
}   // end setLineColour


void AsymmetryView::setArrowColour( double r, double g, double b, double a)
{
    setActorColour( _arrowActor, r, g, b, a);
}   // end setArrowColour


void AsymmetryView::update( const Vec3f &fp, const Vec3f &fq, const Vec3f &fd)
{
    Eigen::Vector3d p = fp.cast<double>();
    Eigen::Vector3d q = fq.cast<double>();
    _line->SetPoint1( &p[0]);
    _line->SetPoint2( &q[0]);
    _line->Update();

    const float nrm = fd.norm();
    const Vec3f d = fd/nrm;
    Mat4f m = Mat4f::Identity();
    m.block<3,1>(0,0) = d;
    m.block<3,1>(0,1) = d.cross( Vec3f(0,0,1));
    m.block<3,1>(0,1).normalize();
    m.block<3,1>(0,2) = m.block<3,1>(0,1).cross( d);
    m.block<3,1>(0,2).normalize();

    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    const Vec3f c = (fp + fq) / 2;
    transform->Translate( &c[0]);
    transform->Concatenate( r3dvis::toVTK( m));
    transform->Scale( nrm, nrm, nrm);
    _transformPD->SetTransform( transform);
    _transformPD->Update();
}   // end update


void AsymmetryView::setLineWidth( double lw)
{
    for ( vtkActor *actor : _actors)
    {
        vtkProperty *prop = actor->GetProperty();
        prop->SetLineWidth( lw);
    }   // end for
}   // end setLineWidth
