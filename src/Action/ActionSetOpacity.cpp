/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
using FaceTools::Action::ActionSetOpacity;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVFlags;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


double ActionSetOpacity::s_minOpacity(0.1);
double ActionSetOpacity::s_maxOpacityOnOverlap(1.0);


ActionSetOpacity::ActionSetOpacity( const QString& dname)
    : FaceAction( dname), _opacitySpinBox(nullptr)
{
    // Whenever this action is notified of an affine change to FVs, run doAction to
    // check for overlaps and top out the maximum opacity to the constructor given value.
    addTriggerEvent( Event::GEOMETRY_CHANGE);
    addTriggerEvent( Event::AFFINE_CHANGE);
    addTriggerEvent( Event::VIEWER_CHANGE);
}   // end ctor


void ActionSetOpacity::postInit()
{
    _opacitySpinBox = new QDoubleSpinBox( static_cast<QWidget*>(parent()));
    _opacitySpinBox->setDecimals(2);
    _opacitySpinBox->setSingleStep(0.01);
    _opacitySpinBox->setRange( s_minOpacity, 1.0);
    _opacitySpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _opacitySpinBox->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    _opacitySpinBox->setValue(1.0);
    connect( _opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ActionSetOpacity::doOnValueChanged);
}   // end postInit


void ActionSetOpacity::setMinOpacity( double v)
{
    s_minOpacity = std::max( 0.1, std::min( v, s_maxOpacityOnOverlap));
}   // end setMinOpacity

void ActionSetOpacity::setOpacityOnOverlap( double v)
{
    s_maxOpacityOnOverlap = std::max( s_minOpacity, std::min( v, 1.0));
}   // end setOpacityOnOverlap


bool ActionSetOpacity::checkEnable( Event)
{
    // Make the opacity spinbox reflect the opacity of the currently selected view.
    bool enabled = false;
    if ( MS::isViewSelected())
    {
        const FV* fv = MS::selectedView();
        _opacitySpinBox->setValue( fv->opacity());
        enabled = true;
    }   // end if
    return enabled;
}   // end checkEnabled


// Check overlap bounds to set opacity.
void ActionSetOpacity::doAction( Event e)
{
    const FMV* fmv = MS::selectedView()->viewer();

    EventGroup outEvent = Event::VIEW_CHANGE;

    if ( EventGroup(e).has(Event::VIEWER_CHANGE))
    {
        // Refresh overlaps in selected view's previous viewer
        assert(MS::isViewSelected());
        fmv = MS::selectedView()->pviewer();
        outEvent.add(Event::ALL_VIEWERS);
    }   // end if

    if ( fmv)
    {
        FVFlags fmap;
        fmv->findOverlaps( fmap);
        fmv->refreshOverlapOpacity( fmap, s_maxOpacityOnOverlap);
    }   // end if

    //emit onEvent(Event(outEvent));
}   // end doAction


void ActionSetOpacity::doOnValueChanged( double newValue)
{
    const FVS& fvs = MS::selectedView()->viewer()->attached();
    for ( FV* fv : fvs)
        fv->setOpacity( newValue);
    //emit onEvent( Event(int(Event::VIEW_CHANGE) | int(Event::ALL_VIEWS)));
    emit onEvent( Event::NONE);
}   // end doOnValueChanged
