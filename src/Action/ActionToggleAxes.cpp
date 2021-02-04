/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <FileIO/FaceModelManager.h>
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
using MS = FaceTools::ModelSelect;
using FMM = FaceTools::FileIO::FaceModelManager;

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
    tp->SetFontSize(10);
    tp->SetOpacity(0.8);
    tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
    tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
}   // end setTextProperties


void setColours( const FMV *vwr, vtkCubeAxesActor *actor)
{
    _setAxesUnits( actor);

    const QColor bg = vwr->backgroundColour();
    //const QColor fg = FaceTools::chooseContrasting( bg);
    const QColor fg = Qt::red;

    setTextProperties( actor->GetLabelTextProperty(0), bg, fg);
    setTextProperties( actor->GetLabelTextProperty(1), bg, fg);
    setTextProperties( actor->GetLabelTextProperty(2), bg, fg);

    setTextProperties( actor->GetTitleTextProperty(0), bg, fg);
    setTextProperties( actor->GetTitleTextProperty(1), bg, fg);
    setTextProperties( actor->GetTitleTextProperty(2), bg, fg);
}   // end setColours

}   // end namespace


ActionToggleAxes::ActionToggleAxes( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _vis(false)
{
    setCheckable( true, _vis);
    addTriggerEvent( Event::LOADED_MODEL | Event::CLOSED_MODEL);
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
    actor->SetLabelOffset( 1.0);
    actor->SetTitleOffset( 1.0);

    actor->GetProperty()->SetOpacity(0.5);

    actor->SetBounds( -200, 200, -200, 200, -200, 200);
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
    actor->SetVisibility(_vis);
    fmv->add( actor);
}   // end _addViewer


bool ActionToggleAxes::update( Event e) { return _vis;}


void ActionToggleAxes::doAction( Event e)
{
    if ( has(e, Event::CLOSED_MODEL))
    {
        if ( FMM::numOpen() == 0)
            _vis = false;
    }   // end if
    else if ( has( e, Event::LOADED_MODEL))
    {
        if ( FMM::numOpen() == 1)
            _vis = true;
    }   // end else if
    else
        _vis = !_vis;
    _refresh();
}   // end doAction


void ActionToggleAxes::_refresh()
{
    for ( const auto& p : _viewers)
    {
        setColours( p.first, p.second);
        p.second->SetVisibility( _vis);
    }   // end for
}   // end _refresh


bool ActionToggleAxes::isAllowed( Event) { return MS::isViewSelected();}
