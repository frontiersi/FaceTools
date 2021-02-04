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

#include <Action/ActionSetOpacity.h>
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
#include <QSignalBlocker>
using FaceTools::Action::ActionSetOpacity;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionSetOpacity::ActionSetOpacity( const QString& dname)
    : FaceAction( dname), _opacitySpinBox(nullptr)
{
    // Event MODEL_SELECT allows the widget to be updated.
    // Event VIEW_CHANGE must be present to allow visualisations that bound
    // a view's min/max opacity to be propagated to the surface.
    addRefreshEvent( Event::VIEWER_CHANGE | Event::AFFINE_CHANGE | Event::VIEW_CHANGE | Event::MODEL_SELECT);
}   // end ctor


void ActionSetOpacity::postInit()
{
    _opacitySpinBox = new QDoubleSpinBox( static_cast<QWidget*>(parent()));
    _opacitySpinBox->setToolTip( toolTip());
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
        _setViewOpacity( fv);
        emit onEvent( Event::NONE);
    }   // end if
}   // end _doOnValueChanged


bool ActionSetOpacity::isAllowed( Event e) { return MS::isViewSelected();}


void ActionSetOpacity::_setViewOpacity( FV *fv)
{
    // Get the base opacity value either directly from the widget (if this is the
    // currently selected view), or as the existing value but accounting for whether
    // or not the value was already reduced from having been found to overlap.
    float v = fv->opacity();
    if ( MS::selectedView() == fv)
        v = _opacitySpinBox->value();
    else if ( _olaps.count(fv) > 0)
        v = _olaps.at(fv);

    // Add back in any old applied visualisation bounding that might no longer apply.
    v = std::min( 1.0f, v + _vbnds[fv]);

    _olaps.erase(fv); // Limit opacity due to overlap
    static const float MAX_OLAP_OPACITY = 0.99f;
    if ( fv->overlaps() && v > MAX_OLAP_OPACITY)
    {
        _olaps[fv] = v;
        v = MAX_OLAP_OPACITY;
    }   // end if

    const float ov = v;
    // Bound the opacity value in consideration of the applied visualisations
    const float maxPoss = std::min<float>( 1.0f, fv->maxAllowedOpacity());
    const float minPoss = std::min<float>( fv->minAllowedOpacity(), maxPoss);
    v = std::min( maxPoss, std::max( v, minPoss));
    _vbnds[fv] = ov - v; // Opacity difference due to visualisations

    fv->setOpacity(v);  // Finally update the applied opacity
}   // end _setViewOpacity


void ActionSetOpacity::_updateOpacities( const FMV *fmv)
{
    for ( FV *fv : fmv->attached())
        _setViewOpacity( fv);
}   // end _updateOpacities


bool ActionSetOpacity::update( Event e)
{
    QSignalBlocker blocker( _opacitySpinBox);
    const FV *fv = MS::selectedView();
    if ( fv)
    {
        if ( has(e, Event::ALL_VIEWERS))
        {
            for ( const FMV *fmv : MS::viewers())
                _updateOpacities( fmv);
        }   // end if
        else
        {
            // Update opacity values on FaceViews in the previous viewer if the viewer was changed
            if ( has( e, Event::VIEWER_CHANGE) && fv->pviewer())
                _updateOpacities( fv->pviewer());
            if ( any( e, Event::VIEWER_CHANGE | Event::AFFINE_CHANGE | Event::VIEW_CHANGE))
                _updateOpacities( fv->viewer());
        }   // end else

        _opacitySpinBox->setValue( _olaps.count(fv) > 0 ? _olaps.at(fv) : fv->opacity());
    }   // end if
    else
        _opacitySpinBox->setValue(1.0);
    return true;
}   // end update


void ActionSetOpacity::purge( const FM *fm)
{
    for ( const FV *fv : fm->fvs())
    {
        _olaps.erase(fv);
        _vbnds.erase(fv);
    }   // end for
}   // end purge
