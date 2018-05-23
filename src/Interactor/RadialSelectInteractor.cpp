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

#include <RadialSelectInteractor.h>
#include <FaceEntryExitInteractor.h>
#include <BoundingVisualisation.h>
#include <FaceModelViewer.h>
#include <FeatureUtils.h>   // RFeatures
#include <cassert>
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::FaceEntryExitInteractor;
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::FaceControl;


// public
RadialSelectInteractor::RadialSelectInteractor( BoundingVisualisation* bv)
    : _bvis(bv), _feei( new FaceEntryExitInteractor), _fc(NULL), _move(false)
{
    connect( _feei, &FaceEntryExitInteractor::onEnterModel, [this](auto fc){ if (_bvis->isApplied(fc)) _fc = fc;});
    connect( _feei, &FaceEntryExitInteractor::onLeaveModel, [this](){ _fc = NULL;});
}   // end ctor


// public
RadialSelectInteractor::~RadialSelectInteractor() { delete _feei;}


// protected
void RadialSelectInteractor::onAttached() { _feei->setViewer(viewer());}
void RadialSelectInteractor::onDetached() { _feei->setViewer(NULL);}


bool RadialSelectInteractor::leftDoubleClick( const QPoint& p)
{
    _move = true;
    return leftDrag(p);
}   // end leftDoubleClick


bool RadialSelectInteractor::leftDrag( const QPoint& p)
{
    if ( !_fc || !_move)
        return false;
    emit onSetNewCentre( _fc, viewer()->project(p));
    return true;
}   // end leftDrag


bool RadialSelectInteractor::leftButtonUp( const QPoint&)
{
    _move = false;
    return false;
}   // end leftButtonUp


bool RadialSelectInteractor::mouseWheelForward( const QPoint& p)
{
    if ( !_fc)
        return false;
    emit onSetNewRadius( _fc, _bvis->radius(_fc)+1);
    return true;
}   // end mouseWheelForward


bool RadialSelectInteractor::mouseWheelBackward( const QPoint& p)
{
    if ( !_fc)
        return false;
    emit onSetNewRadius( _fc, _bvis->radius(_fc)-1);
    return true;
}   // end mouseWheeBackward
