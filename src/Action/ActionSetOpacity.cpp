/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;

namespace {

cv::Vec6d calcSuperBounds( const std::vector<cv::Vec6d>& bset)
{
    cv::Vec6d bounds(DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX);
    for ( const cv::Vec6d& b : bset)
    {
        bounds[0] = std::min(bounds[0], b[0]);
        bounds[1] = std::max(bounds[1], b[1]);
        bounds[2] = std::min(bounds[2], b[2]);
        bounds[3] = std::max(bounds[3], b[3]);
        bounds[4] = std::min(bounds[4], b[4]);
        bounds[5] = std::max(bounds[5], b[5]);
    }   // end for
    return bounds;
}   // end calcSuperBounds


// Return true if the cuboids specified with given edge extents intersect.
bool intersect( const cv::Vec6d& a, const cv::Vec6d& b)
{
    return ((a[0] >= b[0] && a[0] <= b[1]) || (a[1] >= b[0] && a[1] <= b[1])) &&    // Intersection in X axis
           ((a[2] >= b[2] && a[2] <= b[3]) || (a[3] >= b[2] && a[3] <= b[3])) &&    // Intersection in Y axis
           ((a[4] >= b[4] && a[4] <= b[5]) || (a[5] >= b[4] && a[5] <= b[5]));      // Intersection in z axis
}   // end intersect

}   // end namespace


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

    // Whenever this action is notified of an affine change to FaceControls, run doAction to
    // check for overlaps and top out the maximum opacity to the constructor given value.
    addProcessOn( AFFINE_CHANGE);
    addProcessOn( VIEW_CHANGE);
}   // end ctor


double ActionSetOpacity::setOpacityOnOverlap( double v)
{
    _maxOpacityOnOverlap = std::max( _opacitySpinBox->minimum(), std::min( v, 1.0));
    return _maxOpacityOnOverlap;
}   // end setOpacityOnOverlap


bool ActionSetOpacity::testReady( const FaceControl* fc)
{
    // Update the opacity slider to match the opacity of the selected view.
    _opacitySpinBox->setValue( fc->view()->opacity());
    return true;
}   // end testReady


// Action is processed on affine change of the selected FaceControl, so check
// its bounds for overlap against others in the same viewer.
bool ActionSetOpacity::doAction( FaceControlSet& fcsin, const QPoint&)
{
    assert(fcsin.size() == 1);
    // If opacity is already <= _maxOpacityOnOverlap, do nothing.
    if ( _opacitySpinBox->value() <= _maxOpacityOnOverlap)
        return true;

    // Check for overlaps and set the maximum opacity if overlap is found.
    FaceControl* fc = fcsin.first();
    // For efficiency, calculate the superbounds (single cuboid containing all bounds)
    const cv::Vec6d b0 = calcSuperBounds( fc->data()->bounds());

    // Get the other FaceControls in the same viewer
    FaceControlSet fcs = fc->viewer()->attached();
    fcs.erase(fc); // Remove the source

    // If any of the bounds of these others overlap with the source bounds, set
    // the maximum opacity setting for ALL of the other FaceControls in the viewer.
    bool overlap = false;
    for ( FaceControl* f : fcs)
    {
        const cv::Vec6d b1 = calcSuperBounds( f->data()->bounds());
        if ( intersect( b0, b1) || intersect( b1, b0))
        {
            overlap = true;
            break;  // no need to check others
        }   // end if
    }   // end for

    if ( overlap)
    {
        fcs.insert(fc); // Reinsert the source
        _opacitySpinBox->setValue(_maxOpacityOnOverlap);
        setOpacityValue( fcs, _maxOpacityOnOverlap);
    }   // end if

    return true;
}   // end doAction


void ActionSetOpacity::doOnValueChanged( double newValue)
{
    if ( !isEnabled())  // Do nothing if not currently enabled
        return;

    // Change the opacity of the actors for all the FaceControls in the selected FaceControl's viewer.
    const FaceControl* fc = ready1();
    assert(fc);
    setOpacityValue( fc->viewer()->attached(), newValue);
}   // end doOnValueChanged


void ActionSetOpacity::setOpacityValue( const FaceControlSet& fcs, double v)
{
    for ( FaceControl* fc : fcs)
        fc->view()->setOpacity(v);
    ChangeEventSet cset;
    cset.insert(VIEW_CHANGE);
    emit reportFinished( cset, fcs, true);
}   // end setOpacityValue

