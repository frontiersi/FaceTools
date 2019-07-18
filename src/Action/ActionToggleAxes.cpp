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

#include <ActionToggleAxes.h>
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


ActionToggleAxes::ActionToggleAxes( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    setCheckable( true, true);
}   // end ctor


void ActionToggleAxes::postInit()
{
    for ( FMV* fmv : ModelSelector::viewers())
        addViewer(fmv);
}   // end postInit


// private
void ActionToggleAxes::updateAxesUnits( vtkCubeAxesActor* actor) const
{
    const std::string units = FM::LENGTH_UNITS.toStdString();
    actor->SetXUnits( units.c_str());
    actor->SetYUnits( units.c_str());
    actor->SetZUnits( units.c_str());
}   // end updateAxesUnits


namespace {
void setTextProperties( vtkTextProperty* tp, const QColor& bg, const QColor& fg)
{
    tp->SetFontFamilyToCourier();
    tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
    tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
    tp->SetFontSize(27);
}   // end setTextProperties

}   // end namespace


// private
void ActionToggleAxes::addViewer( FMV* fmv)
{
    vtkCubeAxesActor* actor = _viewers[fmv];
    updateAxesUnits( actor);

    actor->SetUseTextActor3D(true);
    //actor->SetUse2DMode(true);

    actor->SetBounds( -200, 200, -200, 200, -200, 200);
    actor->DrawXGridlinesOn();
    actor->DrawYGridlinesOn();
    actor->DrawZGridlinesOn();
    actor->GetProperty()->SetOpacity(0.5);

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

    actor->SetPickable(false);

    fmv->add( actor);
}   // end addViewer


bool ActionToggleAxes::checkState( Event)
{
    const bool chk = isChecked();
    for ( const auto& p : _viewers)
    {
        QColor bg = p.first->backgroundColour();
        QColor fg = chooseContrasting( bg);

        vtkCubeAxesActor* actor = p.second;

        updateAxesUnits( actor);

        setTextProperties( actor->GetLabelTextProperty(0), bg, fg);
        setTextProperties( actor->GetLabelTextProperty(1), bg, fg);
        setTextProperties( actor->GetLabelTextProperty(2), bg, fg);

        setTextProperties( actor->GetTitleTextProperty(0), bg, fg);
        setTextProperties( actor->GetTitleTextProperty(1), bg, fg);
        setTextProperties( actor->GetTitleTextProperty(2), bg, fg);

        actor->SetXAxisTickVisibility( false);
        actor->SetYAxisTickVisibility( false);
        actor->SetZAxisTickVisibility( false);

        actor->SetXAxisMinorTickVisibility( false);
        actor->SetYAxisMinorTickVisibility( false);
        actor->SetZAxisMinorTickVisibility( false);

        actor->SetVisibility(chk);
    }   // end for
    return chk;
}   // end checkState
