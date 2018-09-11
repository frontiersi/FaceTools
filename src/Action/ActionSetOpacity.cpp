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
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::Vis::FV;


ActionSetOpacity::ActionSetOpacity( const QString& dname, double mooo, double minOpacity, QWidget* parent)
    : FaceAction( dname), _opacitySpinBox(new QDoubleSpinBox(parent)), _maxOpacityOnOverlap(mooo)
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
}   // end ctor


double ActionSetOpacity::setOpacityOnOverlap( double v)
{
    _maxOpacityOnOverlap = std::max( _opacitySpinBox->minimum(), std::min( v, 1.0));
    return _maxOpacityOnOverlap;
}   // end setOpacityOnOverlap


bool ActionSetOpacity::testEnabled( const QPoint* mc) const { return ready1() != nullptr;}


void ActionSetOpacity::tellReady( FV* fv, bool v)
{
    _opacitySpinBox->disconnect(this);
    if ( v)
    {
        _opacitySpinBox->setValue( fv->opacity());
        connect( _opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ActionSetOpacity::doOnValueChanged);
    }   // end if
}   // end tellReady


namespace {
bool refreshOpacity( FMV* viewer, FV* fv, double maxOpacityOnOverlap)
{
    FVS fvs = viewer->attached(); // Get the FVs in the same viewer

    // Discover overlaps between pairs of views
    std::vector<FV*> vfvs( fvs.begin(), fvs.end());
    const int n = (int)vfvs.size();

    std::unordered_map<FV*, bool> olaps;
    for ( int i = 0; i < n; ++i)
        olaps[vfvs[i]] = false;

    for ( int i = 0; i < n; ++i)
    {
        FV* ifv = vfvs[i];
        for ( int j = i+1; j < n; ++j)
        {
            FV* jfv = vfvs[j];
            if ( ifv->data()->supersIntersect( *jfv->data()))
                olaps[ifv] = olaps[jfv] = true;
        }   // end for
    }   // end for

    for ( FV* f : fvs)
    {
        const double olapVal = std::min( f->opacity(), maxOpacityOnOverlap);
        f->setOpacity( olaps.at(f) ? olapVal : 1.0);
    }   // end for

    return fv != nullptr ? olaps.at(fv) : false;
}   // end refreshOpacity
}   // end namespace


// Check overlap bounds to set opacity.
bool ActionSetOpacity::doAction( FVS& fvsin, const QPoint&)
{
    assert(fvsin.size() == 1);
    FV* fv = fvsin.first();

    const bool olap = refreshOpacity( fv->viewer(), fv, _maxOpacityOnOverlap);
    fvsin.insert( fv->viewer()->attached());
    if ( fv->pviewer() != nullptr)  // Also refresh the viewer FV came from if this was actioned as a result of VIEWER_CHANGE
    {
        refreshOpacity( fv->pviewer(), nullptr, _maxOpacityOnOverlap);
        fvsin.insert( fv->pviewer()->attached());
    }   // end if

    // Set the value in the spinbox to be for fv (the selected view)
    const double olapVal = std::min( fv->opacity(), _maxOpacityOnOverlap);
    _opacitySpinBox->setValue( olap ? olapVal : 1.0);

    return true;
}   // end doAction


void ActionSetOpacity::doOnValueChanged( double newValue)
{
    if ( !isEnabled())  // Do nothing if not currently enabled
        return;

    assert(gotReady());
    const FVS& fvs = ready().first()->viewer()->attached();
    for ( FV* fv : fvs)
        fv->setOpacity(newValue);
    EventSet cset;
    doAfterAction( cset, fvs, true);
    emit reportFinished( cset, fvs, true);
}   // end doOnValueChanged

