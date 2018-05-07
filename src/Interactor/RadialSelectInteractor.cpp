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
#include <BoundaryVisualisation.h>
#include <FaceModelViewer.h>
#include <FeatureUtils.h>   // RFeatures
#include <cassert>
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::FaceEntryExitInteractor;
using FaceTools::Vis::BoundaryVisualisation;
using FaceTools::FaceControl;


// public
RadialSelectInteractor::RadialSelectInteractor( BoundaryVisualisation* bv)
    : _bvis(bv), _feei( new FaceEntryExitInteractor), _fc(NULL)
{
    connect( _feei, &FaceEntryExitInteractor::onEnterModel, [this](auto fc){ if (_bvis->isApplied(fc)) _fc = fc;});
    connect( _feei, &FaceEntryExitInteractor::onLeaveModel, [this](){ _fc = NULL;});
}   // end ctor


// public
RadialSelectInteractor::~RadialSelectInteractor() { delete _feei;}


// protected
void RadialSelectInteractor::onAttached() { _feei->setViewer(viewer());}
void RadialSelectInteractor::onDetached() { _feei->setViewer(NULL);}


bool RadialSelectInteractor::leftButtonDown( const QPoint& p) { return leftDrag(p);}
bool RadialSelectInteractor::leftDrag( const QPoint& p)
{
    if ( !_fc)
        return false;
    emit onSetNewCentre( _fc, viewer()->project(p));
    return true;
}   // end leftDrag


bool RadialSelectInteractor::rightButtonDown( const QPoint& p) { return rightDrag(p);}
bool RadialSelectInteractor::rightDrag( const QPoint& p)
{
    if ( !_fc)
        return false;

    // New radius as Euclidean distance of point from starting right click drag point.
    cv::Vec3f v = _bvis->centre(_fc);
    cv::Vec3f nv = viewer()->project(p);
    emit onSetNewRadius( _fc, sqrt( RFeatures::l2sq(nv - v)));
    return true;
}   // end rightDrag
