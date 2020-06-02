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

#include <Action/ActionToggleAxes.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <algorithm>
using FaceTools::Action::ActionToggleAxes;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;

namespace {
void _setAxesUnits( vtkCubeAxesActor* actor)
{
    const std::string units = FM::LENGTH_UNITS.toStdString();
    actor->SetXUnits( units.c_str());
    actor->SetYUnits( units.c_str());
    actor->SetZUnits( units.c_str());
}   // end _setAxesUnits


void setTextProperties( vtkTextProperty* tp, const QColor& bg, const QColor& fg)
{
    tp->SetFontFamilyToCourier();
    tp->SetFontSize(12);
    tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
    tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
}   // end setTextProperties


void setColours( const FMV *vwr, vtkCubeAxesActor *actor)
{
    _setAxesUnits( actor);

    QColor bg = vwr->backgroundColour();
    QColor fg = FaceTools::chooseContrasting( bg);

    setTextProperties( actor->GetLabelTextProperty(0), bg, fg);
    setTextProperties( actor->GetLabelTextProperty(1), bg, fg);
    setTextProperties( actor->GetLabelTextProperty(2), bg, fg);

    setTextProperties( actor->GetTitleTextProperty(0), bg, fg);
    setTextProperties( actor->GetTitleTextProperty(1), bg, fg);
    setTextProperties( actor->GetTitleTextProperty(2), bg, fg);
}   // end setColours

}   // end namespace


ActionToggleAxes::ActionToggleAxes( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    setCheckable( true, false);
}   // end ctor


void ActionToggleAxes::postInit()
{
    for ( FMV* fmv : MS::viewers())
        _addViewer(fmv);
}   // end postInit


void ActionToggleAxes::_addViewer( FMV* fmv)
{
    vtkCubeAxesActor* actor = _viewers[fmv];
    actor->SetUseTextActor3D(false);
    actor->SetUse2DMode(true);

    actor->GetProperty()->SetOpacity(0.99);

    actor->SetBounds( -300, 300, -300, 300, -300, 300);
    actor->DrawXGridlinesOn();
    actor->DrawYGridlinesOn();
    actor->DrawZGridlinesOn();

    actor->SetGridLineLocation( vtkCubeAxesActor::VTK_GRID_LINES_FURTHEST);
    actor->SetFlyModeToFurthestTriad();
    actor->SetCamera( const_cast<vtkRenderer*>(fmv->getRenderer())->GetActiveCamera());

    static const cv::Vec3d GCOL( 0.08, 0.4, 0.08);  // rgb

    actor->GetXAxesGridlinesProperty()->SetColor( GCOL[0], GCOL[1], GCOL[2]);
    actor->GetYAxesGridlinesProperty()->SetColor( GCOL[0], GCOL[1], GCOL[2]);
    actor->GetZAxesGridlinesProperty()->SetColor( GCOL[0], GCOL[1], GCOL[2]);

    actor->GetXAxesInnerGridlinesProperty()->SetColor( GCOL[0], GCOL[1], GCOL[2]);
    actor->GetYAxesInnerGridlinesProperty()->SetColor( GCOL[0], GCOL[1], GCOL[2]);
    actor->GetZAxesInnerGridlinesProperty()->SetColor( GCOL[0], GCOL[1], GCOL[2]);

    actor->GetXAxesGridlinesProperty()->SetLineWidth(1);
    actor->GetYAxesGridlinesProperty()->SetLineWidth(1);
    actor->GetZAxesGridlinesProperty()->SetLineWidth(1);

    actor->SetXAxisTickVisibility( false);
    actor->SetYAxisTickVisibility( false);
    actor->SetZAxisTickVisibility( false);

    actor->SetXAxisMinorTickVisibility( false);
    actor->SetYAxisMinorTickVisibility( false);
    actor->SetZAxisMinorTickVisibility( false);

    actor->SetPickable(false);
    fmv->add( actor);
}   // end _addViewer


bool ActionToggleAxes::checkState( Event e)
{
    const bool chk = has( Event::LOADED_MODEL, e) || (isChecked() && MS::isViewSelected());
    for ( const auto& p : _viewers)
    {
        setColours( p.first, p.second);
        p.second->SetVisibility(chk);
    }   // end for
    return chk;
}   // end checkState


bool ActionToggleAxes::isAllowed( Event)
{
    return MS::isViewSelected();
}   // end isAllowed
