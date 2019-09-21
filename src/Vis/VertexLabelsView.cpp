/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <Vis/VertexLabelsView.h>
#include <vtkPointData.h>
using FaceTools::Vis::VertexLabelsView;


vtkSmartPointer<vtkPolyData> VertexLabelsView::createLabels( const RFeatures::ObjModel& model) const
{
    vtkSmartPointer<vtkPolyData> pdata = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIntArray> vlabels = vtkSmartPointer<vtkIntArray>::New();

    const IntSet& vids = model.vtxIds();
    const int nv = static_cast<int>(vids.size());
    points->SetNumberOfPoints( nv);
    vlabels->SetNumberOfValues( nv);
    vlabels->SetName( "vertexIds");

    int i = 0;
    for ( int vid : vids)
    {
        vlabels->SetValue( i, vid);
        points->SetPoint( i, &model.vtx(vid)[0]);
        vertices->InsertNextCell(1);
        vertices->InsertCellPoint(i++);
    }   // end for

    pdata->SetVerts( vertices);
    pdata->SetPoints( points);
    pdata->GetPointData()->AddArray( vlabels);
    return pdata;
}   // end createLabels
