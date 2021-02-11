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

#include <Action/ActionToggleLegend.h>
#include <Vis/ColourVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <vtkTextProperty.h>
using FaceTools::Action::ActionToggleLegend;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::ColourVisualisation;
using FaceTools::Vis::FV;
using FMV = FaceTools::FaceModelViewer;
using MS = FaceTools::ModelSelect;


ActionToggleLegend::ActionToggleLegend( const QString& dn) : FaceAction( dn)
{
    addRefreshEvent( Event::VIEW_CHANGE | Event::VIEWER_CHANGE | Event::SURFACE_DATA_CHANGE);
    setCheckable(true,true);
}   // end ctor


void ActionToggleLegend::postInit()
{
    /*
    for ( FMV *fmv : MS::viewers())
    {
        r3dvis::ScalarLegend *sleg = &_legends[fmv];
        connect( fmv, &ModelViewer::onResized, [=]()
        {
            this->_recalcLegendPlacement(sleg);
            sleg->prop()->RenderOpaqueGeometry(fmv->getRenderer());
        });
    }   // end for
    */
    ; // no-op
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
    tp->SetFontSize(13);
    tp->SetBold(true);
    tp->SetItalic(false);
}   // end updateTitleProperties

void updateLabelProperties( vtkTextProperty* tp)
{
    tp->SetFontFamilyToCourier();
    tp->SetItalic(false);
    tp->SetBold(false);
    tp->SetFontSize(12);
}   // end updateLabelProperties


bool updateLegend( r3dvis::ScalarLegend &sleg, const FV *fv)
{
    const FMV *fmv = fv->viewer();
    const ColourVisualisation *svis = fv->activeColours();

    bool isVis = false;
    // If svis is not null, it doesn't mean that it should remain so.
    // We check availability since the action that checks whether the
    // visualisation should still be applied may not have been called yet.
    if ( svis && svis->isAvailable(fv))
    {
        isVis = true;

        // Set the legend title and colours lookup table for the scalar legend.
        sleg.setTitle( svis->label().toStdString());
        const QColor bg = fmv->backgroundColour();
        const QColor fg = FaceTools::chooseContrasting( bg);
        updateTitleProperties( sleg.titleProperty());
        updateLabelProperties( sleg.labelProperty());
        setTextColours( sleg.titleProperty(), bg, fg);
        setTextColours( sleg.labelProperty(), bg, fg);

        sleg.setLookupTable( svis->lookupTable());
        sleg.setNumLabels( std::min( int(svis->numColours()) + 1, 25));
    }   // end if

    return isVis;
}   // end updateLegend

}   // end namespace


void ActionToggleLegend::_recalcLegendPlacement( r3dvis::ScalarLegend *sleg)
{
    // Setting the position and height here forces the texture to be recalculated.
    // NB on high DPI screens, the texture is not stable - bug in VTK.
    sleg->setPosition( 0.8, 0.50);
    sleg->setWidth( 0.1);
    sleg->setHeight( 0.47);
}   // end _recalcLegendPlacement


bool ActionToggleLegend::isAllowed( Event)
{
    // Toggling is allowed if the selected view has an active surface
    const FV *fv = MS::selectedView();
    return fv && fv->activeColours();
}   // end isAllowed


bool ActionToggleLegend::update( Event e)
{
    const FV *sfv = MS::selectedView();
    const bool isVis = sfv && isChecked();

    for ( FMV *fmv : MS::viewers())
    {
        const FV *fv = nullptr;
        if ( sfv)
        {
            // Only show legend in viewer for currently selected model
            // or if the viewer only has a single view.
            fv = fmv->get( sfv->data());
            if ( !fv && fmv->attached().size() == 1)
                fv = fmv->selected();
        }   // end if

        r3dvis::ScalarLegend &sleg = _legends[fmv];
        if ( isVis && fv && updateLegend( sleg, fv))
            fmv->add( sleg.prop());
        else
            fmv->remove( sleg.prop());
        _recalcLegendPlacement( &sleg);
    }   // end for

    return isChecked();
}   // end update
