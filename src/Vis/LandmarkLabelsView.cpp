/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#include <Vis/LandmarkLabelsView.h>
#include <vtkPointData.h>
#include <vtkStringArray.h>
#include <LndMrk/LandmarksManager.h>
#include <LndMrk/LandmarkSet.h>
#include <FaceModel.h>
using FaceTools::Vis::LandmarkLabelsView;
using FaceTools::FM;
using LMAN = FaceTools::Landmark::LandmarksManager;


vtkSmartPointer<vtkPolyData> LandmarkLabelsView::createLabels( const FM *fm) const
{
    const Landmark::LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    const IntSet& lmids = lmks.ids();
    std::vector<std::string> lmnames;
    std::vector<r3d::Vec3d> lmpos;

    for ( int lmid : lmids)
    {
        if ( LMAN::isBilateral( lmid))
        {
            lmnames.push_back( LMAN::makeLandmarkString( lmid, FaceLateral::FACE_LATERAL_LEFT).toStdString());
            lmpos.push_back( lmks.pos( lmid, FaceLateral::FACE_LATERAL_LEFT).cast<double>());
            lmnames.push_back( LMAN::makeLandmarkString( lmid, FaceLateral::FACE_LATERAL_RIGHT).toStdString());
            lmpos.push_back( lmks.pos( lmid, FaceLateral::FACE_LATERAL_RIGHT).cast<double>());
        }   // end if
        else
        {
            lmnames.push_back( LMAN::makeLandmarkString( lmid, FaceLateral::FACE_LATERAL_MEDIAL).toStdString());
            lmpos.push_back( lmks.pos( lmid, FaceLateral::FACE_LATERAL_MEDIAL).cast<double>());
        }   // end else
    }   // end for

    vtkSmartPointer<vtkPolyData> pdata = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkStringArray> labels = vtkSmartPointer<vtkStringArray>::New();

    const int n = static_cast<int>( lmnames.size());
    points->SetNumberOfPoints( n);
    labels->SetNumberOfValues( n);
    labels->SetName( "landmarkLabels");

    for ( int i = 0; i < n; ++i)
    {
        labels->SetValue( i, lmnames[i].c_str());
        points->SetPoint( i, &lmpos[i][0]);
        cells->InsertNextCell(1);
        cells->InsertCellPoint(i);
    }   // end for

    pdata->SetVerts( cells);
    pdata->SetPoints( points);
    pdata->GetPointData()->AddArray( labels);
    return pdata;
}   // end createLabels


bool LandmarkLabelsView::canCreateLabels( const FM *fm) const
{
    return fm && fm->hasLandmarks();
}   // end canCreateLabels
