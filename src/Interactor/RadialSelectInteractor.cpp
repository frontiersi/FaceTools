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

#include <RadialSelectInteractor.h>
#include <LoopSelectVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::LoopSelectVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;


const QString RadialSelectInteractor::s_msg( tr("Reposition radial area by left-click and dragging the centre; change radius using the mouse wheel."));


// public
RadialSelectInteractor::RadialSelectInteractor( MEEI* meei, LoopSelectVisualisation* vis, QStatusBar* sbar)
    : ModelViewerInteractor( nullptr, sbar), _meei(meei), _vis(vis), _move(false), _onReticule(false)
{
    connect( meei, &MEEI::onEnterProp, [this](FV* fv, const vtkProp* p){ if ( _vis->belongs( p, fv)) this->doOnEnterReticule();});
    connect( meei, &MEEI::onLeaveProp, [this](FV* fv, const vtkProp* p){ if ( _vis->belongs( p, fv)) this->doOnLeaveReticule();});
    setEnabled(false);
}   // end ctor


void RadialSelectInteractor::doOnEnterReticule()
{
    _onReticule = true;
}   // end doOnEnterReticule


void RadialSelectInteractor::doOnLeaveReticule()
{
    _onReticule = false;
}   // end doOnLeaveReticule


bool RadialSelectInteractor::leftButtonDown( const QPoint& p)
{
    _move = _onReticule;
    /*
    if ( _move)
        _vis->setPickable( _meei->model(), false); // So projecting points onto the face works
    */
    return _onReticule;
}   // end leftButtonDown


bool RadialSelectInteractor::leftButtonUp( const QPoint& p)
{
    //_vis->setPickable( _meei->model(), true);
    _move = false;
    return mouseMove(p);
}   // end leftButtonUp


// Left drag (but only after _move flag set with leftButtonDown)
// to reposition the centre of the boundary.
bool RadialSelectInteractor::leftDrag( const QPoint& p)
{
    FV* fv = _meei->model();
    if ( fv && _move)
    {
        cv::Vec3f v;
        if ( fv->projectToSurface( p, v))
            emit onSetCentre( fv, v);
    }   // end if
    return _move;
}   // end leftDrag


// Increase the radius
bool RadialSelectInteractor::mouseWheelForward( const QPoint& p)
{
    if ( _onReticule)
        emit onIncreaseRadius( _meei->model());
    return _onReticule;
}   // end mouseWheelForward


// Decrease the radius
bool RadialSelectInteractor::mouseWheelBackward( const QPoint& p)
{
    if ( _onReticule)
        emit onDecreaseRadius( _meei->model());
    return _onReticule;
}   // end mouseWheeBackward


bool RadialSelectInteractor::mouseMove( const QPoint& p)
{
    FV* fv = _meei->model();
    if ( fv)
    {
        const FM* fm = fv->data();
        for ( FV* f : fm->fvs())
            _vis->setHighlighted( f, _onReticule);
        fm->updateRenderers();
    }   // end if
    return false;
}   // end mouseMove


// protected
void RadialSelectInteractor::onEnabledStateChanged( bool v)
{
    if ( v)
        showStatus( s_msg, 10000);    // 10 seconds display time
    else
        clearStatus();
}   // end onEnabledStateChanged
