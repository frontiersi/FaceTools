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

#include <ActionSetOpacity.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
using FaceTools::Action::ActionSetOpacity;
using FaceTools::Action::FaceAction;
using FaceTools::FVFlags;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::Vis::FV;


ActionSetOpacity::ActionSetOpacity( const QString& dname, double mooo, double minOpacity, QWidget* parent)
    : FaceAction( dname), _maxOpacityOnOverlap(mooo), _opacitySpinBox(new QDoubleSpinBox(parent)), _viewer(nullptr)
{
    _opacitySpinBox->setDecimals(2);
    _opacitySpinBox->setSingleStep(0.05);
    minOpacity = std::max( 0.0, std::min( minOpacity, 1.0));
    _opacitySpinBox->setRange( minOpacity, 1.0);
    _opacitySpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _opacitySpinBox->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    _opacitySpinBox->setValue(1.0);

    setOpacityOnOverlap( mooo); // Ensure fixed in correct range

    connect( _opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ActionSetOpacity::doOnValueChanged);

    // Whenever this action is notified of an affine change to FVs, run doAction to
    // check for overlaps and top out the maximum opacity to the constructor given value.
    setRespondToEvent( GEOMETRY_CHANGE);
    setRespondToEvent( AFFINE_CHANGE);
    setRespondToEvent( VIEWER_CHANGE);
    setRespondToEvent( VIEW_CHANGE);
}   // end ctor


double ActionSetOpacity::setOpacityOnOverlap( double v)
{
    _maxOpacityOnOverlap = std::max( _opacitySpinBox->minimum(), std::min( v, 1.0));
    _opacitySpinBox->setValue( _maxOpacityOnOverlap);
    return _maxOpacityOnOverlap;
}   // end setOpacityOnOverlap


bool ActionSetOpacity::testEnabled( const QPoint*) const { return _viewer != nullptr;}


void ActionSetOpacity::tellReady( const FV* fv, bool v)
{
    // Make the opacity spinbox reflect the opacity of the current view.
    disconnect( _opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ActionSetOpacity::doOnValueChanged);
    if ( v)
    {
        _opacitySpinBox->setValue( fv->opacity());
        connect( _opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ActionSetOpacity::doOnValueChanged);
        _viewer = fv->viewer();
    }   // end if
}   // end tellReady


namespace {
void checkForMetricVisualise( FVFlags& fmap)
{
    std::vector<const FV*> cfvs;
    std::for_each( std::begin(fmap), std::end(fmap), [&cfvs](const std::pair<const FV*, bool>& p){ if ( p.first->hasMetricVisualiser()) cfvs.push_back(p.first);});
    std::for_each( std::begin(cfvs), std::end(cfvs), [&fmap](const FV* fv){ fmap[fv] = true;});
}   // end checkForMetricVisualise
}   // end namespace


// Check overlap bounds to set opacity.
bool ActionSetOpacity::doAction( FVS& fvs, const QPoint&)
{
    assert(_viewer);

    FVFlags fmap; // Data structure to record which views should have their opacity lowered.
    _viewer->findOverlaps( fmap); // Find overlaps in fv's viewer
    checkForMetricVisualise( fmap);
    _viewer->refreshOverlapOpacity( fmap, _maxOpacityOnOverlap);   // Set overlaps according to fmap

    // Of the identified FaceViews, get their previous viewers.
    // Refresh overlaps in these viewers (could've been actioned as a result of VIEWER_CHANGE)
    FMVS fmvs;
    for ( const auto& p : fmap)
        fmvs.insert( p.first->pviewer());
    fmvs.erase(nullptr);

    for ( FMV* fmv : fmvs)
    {
        fmap.clear();
        fmv->findOverlaps( fmap);
        checkForMetricVisualise( fmap);
        fmv->refreshOverlapOpacity( fmap, _maxOpacityOnOverlap);
    }   // end for

    // Record the changed views.
    for ( const auto& p : fmap)
    {
        if ( p.second)
            fvs.insert( const_cast<FV*>(p.first));
    }   // end for

    return true;
}   // end doAction


void ActionSetOpacity::doOnValueChanged( double newValue)
{
    if ( !isEnabled())
        return;

    const FVS& fvs = _viewer->attached();
    for ( FV* fv : fvs)
        fv->setOpacity( newValue);
    EventSet cset;
    doAfterAction( cset, fvs, true);
    emit reportFinished( cset, fvs, true);
}   // end doOnValueChanged
