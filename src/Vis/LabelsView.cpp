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

#include <Vis/LabelsView.h>
#include <ModelViewer.h>
#include <FaceModel.h>
#include <r3dvis/VtkTools.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkTextProperty.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
using FaceTools::Vis::LabelsView;
using FaceTools::ModelViewer;
using FaceTools::FM;
using FaceTools::Mat4f;


void LabelsView::refresh( const FM *fm)
{
    vtkSmartPointer<vtkPolyData> pdata = createLabels( fm);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData( pdata);
    _actor->SetMapper(mapper);

    _filter->SetInputData( pdata);
    _filter->SetLabelArrayName( pdata->GetPointData()->GetArrayName(0));
    vtkSmartPointer<vtkLabelPlacementMapper> lmapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
    lmapper->SetInputConnection( _filter->GetOutputPort());
    lmapper->SetPlaceAllLabels( placeAllLabels());
    _labels->SetMapper( lmapper);

    _moving = false;
    _lt = Mat4f::Identity();
    transform( fm->transformMatrix());
}   // end refresh


LabelsView::LabelsView( int fontSize, bool boldOn)
    : _actor( vtkSmartPointer<vtkActor>::New()),
      _labels( vtkSmartPointer<vtkActor2D>::New()),
      _filter( vtkSmartPointer<vtkPointSetToLabelHierarchy>::New()),
      _visible( false),
      _moving( false),
      _lt( Mat4f::Identity())
{
    _actor->GetProperty()->SetRepresentationToPoints();
    _actor->GetProperty()->SetPointSize(2);
    _actor->GetProperty()->SetAmbient(1.0);
    _actor->GetProperty()->SetDiffuse(0.0);
    _actor->GetProperty()->SetSpecular(0.0);
    _actor->SetPickable(false);

    _labels->SetPickable(false);

    vtkTextProperty* tp = _filter->GetTextProperty();
    tp->SetFontFamilyToCourier();
    tp->SetFontSize( fontSize);
    tp->SetBold( boldOn);
}   // end ctor


void LabelsView::setColours( const QColor& fg, const QColor& bg)
{
    _actor->GetProperty()->SetColor( fg.redF(), fg.greenF(), fg.blueF());
    vtkTextProperty *tp = _filter->GetTextProperty();
    tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
    tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
}   // end setColours


void LabelsView::setVisible( bool v, ModelViewer* vwr)
{
    vwr->remove(_actor);
    vwr->remove(_labels);
    if ( v)
    {
        vwr->add(_actor);
        vwr->add(_labels);
    }   // end if
    _visible = v;
}   // end setVisible


void LabelsView::transform( const Mat4f& m)
{
    r3dvis::fixTransform( _actor, r3dvis::toVTK( m * _lt.inverse()));
    _moving = false;
    _lt = m;    // Store last transform applied
}   // end transform
