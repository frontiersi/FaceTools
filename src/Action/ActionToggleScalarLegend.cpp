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

#include <ActionToggleScalarLegend.h>
#include <SurfaceMetricsMapper.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <vtkTextProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionToggleScalarLegend;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::SurfaceMetricsMapper;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using MS = FaceTools::Action::ModelSelector;


ActionToggleScalarLegend::ActionToggleScalarLegend( const QString& dn)
    : FaceAction( dn)
{
    setCheckable(true,true);
    for ( FMV* fmv : MS::viewers())
        _legends[fmv] = new RVTK::ScalarLegend( fmv->getRenderWindow()->GetInteractor());
}   // end ctor


ActionToggleScalarLegend::~ActionToggleScalarLegend()
{
    for ( const auto& p : _legends)
        delete p.second;
}   // end dtor

namespace  {

void setTextColours( vtkTextProperty* tp, const QColor& bg, const QColor& fg)
{
    tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
    tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
}   // end setTextColours

void updateTitleProperties( vtkTextProperty* tp)
{
    tp->SetFontFamilyToCourier();
    tp->SetFontSize(16);
    tp->SetBold(true);
    tp->SetItalic(false);
}   // end updateTitleProperties

void updateLabelProperties( vtkTextProperty* tp)
{
    tp->SetFontFamilyToCourier();
    tp->SetItalic(false);
    tp->SetBold(false);
    tp->SetFontSize(14);
}   // end updateLabelProperties

}   // end namespace


bool ActionToggleScalarLegend::checkState( Event)
{
    for ( const auto& p : _legends)
    {
        RVTK::ScalarLegend* legend = p.second;
        bool showLegend = false;

        if ( isChecked())
        {
            SurfaceMetricsMapper *smm = nullptr;
            for ( FV *fv : p.first->attached())
            {
                if ( (smm = fv->activeSurface()))
                    break;
            }   // end for

            if ( smm && smm->isScalarMapping())
            {
                // Set the legend title and colours lookup table for the scalar legend.
                legend->setTitle( smm->label());
                legend->setLookupTable( smm->scalarLookupTable());
                showLegend = true;
            }   // end if
        }   // end if

        updateTitleProperties( legend->titleProperty());
        updateLabelProperties( legend->labelProperty());

        const QColor bg = p.first->backgroundColour();
        const QColor fg = chooseContrasting( bg);
        setTextColours( legend->titleProperty(), bg, fg);
        setTextColours( legend->labelProperty(), bg, fg);

        legend->setVisible( showLegend);
    }   // end for

    return isChecked();
}   // end checkChecked


bool ActionToggleScalarLegend::checkEnable( Event)
{
    for ( const auto& p : _legends)
        for ( const FV *fv : p.first->attached())
            if ( fv->activeSurface() && fv->activeSurface()->isScalarMapping())
                return true;
    return false;
}   // end checkEnabled
