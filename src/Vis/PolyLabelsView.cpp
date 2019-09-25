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

#include <Vis/PolyLabelsView.h>
#include <vtkPointData.h>
#include <cassert>
using FaceTools::Vis::PolyLabelsView;


vtkSmartPointer<vtkPolyData> PolyLabelsView::createLabels( const RFeatures::ObjModel& model) const
{
    vtkSmartPointer<vtkPolyData> pdata = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIntArray> vlabels = vtkSmartPointer<vtkIntArray>::New();

    assert( model.hasSequentialFaceIds());

    const int np = model.numPolys();
    points->SetNumberOfPoints( np);
    vlabels->SetNumberOfValues( np);
    vlabels->SetName( "polyIds");

    for ( int i = 0; i < np; ++i)
    {
        vlabels->SetValue( i, i);

        const int* fvidxs = model.fvidxs(i);
        cv::Vec3d mpos = (model.uvtx(fvidxs[0]) +
                          model.uvtx(fvidxs[1]) +
                          model.uvtx(fvidxs[2])) * 1.0/3;

        points->SetPoint( i, &mpos[0]);
        vertices->InsertNextCell(1);
        vertices->InsertCellPoint(i);
    }   // end for

    pdata->SetVerts( vertices);
    pdata->SetPoints( points);
    pdata->GetPointData()->AddArray( vlabels);
    return pdata;
}   // end createLabels


void PolyLabelsView::setColours( const QColor& fg, const QColor& bg)
{
    const QColor nfg = QColor::fromRgbF( 0.5 * fg.redF(), 0.5 * fg.greenF(), 0.5 * fg.blueF() + 0.5);
    LabelsView::setColours( nfg, bg);
}   // end setColours
