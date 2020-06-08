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

#include <Vis/MaskView.h>
#include <Vis/FaceView.h>
#include <r3dvis/VtkActorCreator.h>
#include <vtkPointData.h>
using FaceTools::Vis::MaskView;
using FaceTools::Vis::FV;


MaskView::MaskView( const r3d::Mesh &mesh) : SimpleView()
{
    vtkSmartPointer<vtkActor> actor = r3dvis::VtkActorCreator::generateSurfaceActor( mesh);
    actor->SetPickable( false);

    // Set curvature data
    const r3d::Curvature cv( const_cast<r3d::Mesh&>(mesh)); // Have to do this for now - should make a one shot version...
    vtkSmartPointer<vtkFloatArray> nrms = r3dvis::makeNormals( cv);
    nrms->SetName("Normals");
    r3dvis::getPolyData(actor)->GetPointData()->SetNormals(nrms);

    initActor(actor);   // Stores it
}   // end ctor


void MaskView::refresh( const FV *fv)
{
    vtkProperty *prop = _actors[0]->GetProperty();
    prop->SetBackfaceCulling(true);
    prop->SetAmbient( 0.0);
    prop->SetDiffuse( 1.0);
    prop->SetSpecular( 0.0);
    prop->SetOpacity( 0.999);

    const QColor col = fv->colour();
    prop->SetColor( col.blueF(), col.redF(), col.greenF()); // Swap red and blue for contrast
    if ( FV::smoothLighting())
        prop->SetInterpolationToPhong();
    else
        prop->SetInterpolationToFlat();
}   // end refresh
