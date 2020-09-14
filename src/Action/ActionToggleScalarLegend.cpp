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

#include <Action/ActionToggleScalarLegend.h>
#include <Vis/ScalarVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <vtkTextProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionToggleScalarLegend;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using MS = FaceTools::Action::ModelSelector;


ActionToggleScalarLegend::ActionToggleScalarLegend( const QString& dn)
    : FaceAction( dn)
{
    setCheckable(true,true);
    addRefreshEvent( Event::VIEW_CHANGE);
}   // end ctor


void ActionToggleScalarLegend::postInit()
{
    for ( FMV* fmv : MS::viewers())
        _legends[fmv] = new r3dvis::ScalarLegend( fmv->getRenderWindow()->GetInteractor());
}   // end postInit


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


bool ActionToggleScalarLegend::update( Event)
{
    for ( const auto& p : _legends)
    {
        const FMV *fmv = p.first;
        r3dvis::ScalarLegend* legend = p.second;
        bool showLegend = false;
        legend->setTitle( "NO TITLE");

        if ( isChecked())
        {
            // Scalar visualisations are exclusive so get the first view with one
            // and create and set the lookup table.
            ScalarVisualisation *svis = nullptr;
            for ( FV *fv : fmv->attached())
                if ( (svis = fv->activeScalars()))
                    break;

            if ( svis)
            {
                // Set the legend title and colours lookup table for the scalar legend.
                legend->setLookupTable( svis->lookupTable( fmv->getRenderer()));
                legend->setTitle( svis->label().toStdString());
                legend->setNumLabels( int(svis->numColours()) + 1);
                showLegend = true;
            }   // end if
        }   // end if

        legend->setVisible( showLegend);

        const QColor bg = fmv->backgroundColour();
        const QColor fg = chooseContrasting( bg);
        updateTitleProperties( legend->titleProperty());
        updateLabelProperties( legend->labelProperty());
        setTextColours( legend->titleProperty(), bg, fg);
        setTextColours( legend->labelProperty(), bg, fg);
    }   // end for

    return isChecked();
}   // end checkChecked


bool ActionToggleScalarLegend::isAllowed( Event)
{
    for ( const auto& p : _legends)
        for ( const FV *fv : p.first->attached())
            if ( fv->activeScalars())
                return true;
    return false;
}   // end isAllowed
