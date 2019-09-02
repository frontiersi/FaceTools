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

#include <PolyLabelsVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <vtkPointData.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::PolyLabelsVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Action::Event;

namespace {

vtkSmartPointer<vtkPolyData> createLabels( const RFeatures::ObjModel& model, const std::string& nm)
{
    vtkSmartPointer<vtkPolyData> pdata = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIntArray> vlabels = vtkSmartPointer<vtkIntArray>::New();

    const IntSet& fids = model.faces();
    const int np = static_cast<int>(fids.size());
    points->SetNumberOfPoints( np);
    vlabels->SetNumberOfValues( np);
    vlabels->SetName( nm.c_str());

    int i = 0;
    for ( int fid : fids)
    {
        vlabels->SetValue( i, fid);
        cv::Vec3d mpos = (model.vtx(model.fvidxs(fid)[0]) + 
                         model.vtx(model.fvidxs(fid)[1]) + 
                         model.vtx(model.fvidxs(fid)[2])) * 1.0/3;

        points->SetPoint( i, &mpos[0]);
        vertices->InsertNextCell(1);
        vertices->InsertCellPoint(i++);
    }   // end for

    pdata->SetVerts( vertices);
    pdata->SetPoints( points);
    pdata->GetPointData()->AddArray( vlabels);
    return pdata;
}   // end createLabels


bool nearIdentity( const cv::Matx44d& d, size_t ndp=8)
{
    const cv::Matx44d I = cv::Matx44d::eye();
    for ( int i = 0; i < 16; ++i)
    {
        if ( (RFeatures::roundndp(d.val[i], ndp) - I.val[i]) != 0.0)
            return false;
    }   // end for
    return true;
}   // end nearIdentity

}   // end namespace


PolyLabelsVisualisation::~PolyLabelsVisualisation()
{
    while (!_views.empty())
        purge( const_cast<FV*>(_views.begin()->first), Event::NONE);
}   // end dtor


void PolyLabelsVisualisation::apply( FV* fv, const QPoint*)
{
    if ( _views.count(fv) == 0)
    {
        LabelsView lv;

        lv.actor = vtkSmartPointer<vtkActor>::New();
        lv.actor->GetProperty()->SetRepresentationToPoints();
        lv.actor->GetProperty()->SetPointSize(2);
        lv.actor->GetProperty()->SetAmbient(1.0);
        lv.actor->GetProperty()->SetDiffuse(0.0);
        lv.actor->GetProperty()->SetSpecular(0.0);
        lv.actor->SetPickable(false);

        lv.labels = vtkSmartPointer<vtkActor2D>::New();
        lv.labels->SetPickable(false);

        lv.filter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
        vtkTextProperty* tp = lv.filter->GetTextProperty();
        tp->SetFontFamilyToCourier();
        tp->SetBold(true);
        tp->SetFontSize(19);

        _views[fv] = lv;
        syncActorsToData(fv);
    }   // end if
}   // end apply


bool PolyLabelsVisualisation::purge( FV* fv, Event)
{
    setVisible(fv, false);
    if (_views.count(fv) > 0)
        _views.erase(fv);
    return true;
}   // end purge


void PolyLabelsVisualisation::setVisible( FV* fv, bool v)
{
    if (_views.count(fv) > 0)
    {
        if ( v)
        {
            fv->viewer()->add(_views.at(fv).actor);
            fv->viewer()->add(_views.at(fv).labels);
        }   // end if
        else
        {
            fv->viewer()->remove(_views.at(fv).actor);
            fv->viewer()->remove(_views.at(fv).labels);
        }   // end else

        _views.at(fv).visible = v;
    }   // end if
}   // end setVisible


bool PolyLabelsVisualisation::isVisible( const FV* fv) const
{
    bool vis = false;
    if (_views.count(fv) > 0)
        vis = _views.at(fv).visible;
    return vis;
}   // end isVisible


void PolyLabelsVisualisation::syncActorsToData( const FV *fv, const cv::Matx44d &d)
{
    if ( _views.count(fv) == 0)
        return;

    LabelsView& lv = _views.at(fv);
    if ( !nearIdentity( d))
    {
        const bool wasVisible = isVisible(fv);
        setVisible( const_cast<FV*>(fv), false);
        lv.visible = wasVisible;
    }   // end if
    else
    {
        static const std::string labelsName = "polyIds";
        vtkSmartPointer<vtkPolyData> pdata = createLabels( fv->data()->model(), labelsName);

        lv.filter->SetInputData( pdata);
        lv.filter->SetLabelArrayName( labelsName.c_str());
        vtkSmartPointer<vtkLabelPlacementMapper> lmapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
        lmapper->SetInputConnection( lv.filter->GetOutputPort());
        lv.labels->SetMapper( lmapper);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData( pdata);
        lv.actor->SetMapper(mapper);

        setVisible( const_cast<FV*>(fv), isVisible(fv));
    }   // end else
}   // end syncActorsToData


void PolyLabelsVisualisation::checkState( const FV* fv)
{
    QColor bg = fv->viewer()->backgroundColour();
    QColor fg = chooseContrasting( bg);
    _views.at(fv).actor->GetProperty()->SetColor( fg.redF(), fg.greenF(), fg.blueF());
    vtkTextProperty* tp = _views.at(fv).filter->GetTextProperty();
    tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
    tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
}   // end checkState
