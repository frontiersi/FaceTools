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

#include <Action/ActionSetOpacity.h>
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
using FaceTools::Action::ActionSetOpacity;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;

float ActionSetOpacity::s_opacityReduction(0.0f);
void ActionSetOpacity::setOverlapOpacityReduction( float v) { s_opacityReduction = v;}


ActionSetOpacity::ActionSetOpacity( const QString& dname)
    : FaceAction( dname), _opacitySpinBox(nullptr)
{
}   // end ctor


void ActionSetOpacity::postInit()
{
    _opacitySpinBox = new QDoubleSpinBox( static_cast<QWidget*>(parent()));
    _opacitySpinBox->setValue(1.0);
    _opacitySpinBox->setDecimals(1);
    _opacitySpinBox->setSingleStep(0.1);
    _opacitySpinBox->setRange( 0.1, 1.0);
    _opacitySpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _opacitySpinBox->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    connect( _opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ActionSetOpacity::_doOnValueChanged);
}   // end postInit


void ActionSetOpacity::_doOnValueChanged()
{
    FV *fv = MS::selectedView();
    if ( fv)
    {
        _setOpacity( fv);
        emit onEvent( Event::NONE);
    }   // end if
}   // end _doOnValueChanged


void ActionSetOpacity::_setOpacity( FV *fv)
{
    const float maxIfOverlap = fv->overlaps() ? 1.0f - s_opacityReduction : 1.0f;
    // maxPoss is the maximum given overlap and the visualisations on the view.
    const float maxPoss = std::min<float>( maxIfOverlap, fv->maxAllowedOpacity());
    // Get the possible range that opacity can be set within
    const float minPoss = std::min<float>( fv->minAllowedOpacity(), maxPoss);

    const double uiRange = _opacitySpinBox->maximum() - _opacitySpinBox->minimum();
    const float vprop = (_opacitySpinBox->value() - _opacitySpinBox->minimum()) / uiRange;
    const float v = float( vprop * (maxPoss - minPoss) + minPoss);
    fv->setOpacity( v);
}   // end _setOpacity


void ActionSetOpacity::_updateOpacities( const FMV *fmv)
{
    for ( FV *fv : fmv->attached())
        _setOpacity( fv);
}   // end _updateOpacities


bool ActionSetOpacity::checkState( Event e)
{
    const FV *fv = MS::selectedView();
    if ( fv)
    {
        if ( has(e, Event::ALL_VIEWERS))
        {
            for ( const FMV *fmv : MS::viewers())
                for ( FV *f : fmv->attached())
                    _setOpacity(f);
        }   // end if
        else
        {
            // Update opacity values on FaceViews in the previous viewer if the viewer was changed
            if ( has( e, Event::VIEWER_CHANGE) && fv->pviewer())
                _updateOpacities( fv->pviewer());
            if ( has( e, Event::VIEWER_CHANGE | Event::AFFINE_CHANGE | Event::VIEW_CHANGE))
                _updateOpacities( fv->viewer());
        }   // end else
    }   // end if
    else
        _opacitySpinBox->setValue(1.0);
    return true;
}   // end checkState


bool ActionSetOpacity::isAllowed( Event) { return MS::isViewSelected();}
