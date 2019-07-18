/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionResetCamera.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
using FaceTools::Action::ActionResetCamera;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FMV;
using FaceTools::FVS;
using MS = FaceTools::Action::ModelSelector;


ActionResetCamera::ActionResetCamera( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    addTriggerEvent( Event::VIEWER_CHANGE);
}   // end ctor

void ActionResetCamera::resetCamera( const FV* fv)
{
    FMV* vwr = fv->viewer();
    const FM* fm = fv->data();
    fm->lockForRead();
    const RFeatures::ObjModelBounds& bounds = *fm->bounds()[0];
    vwr->resetDefaultCamera( static_cast<float>( 1.5 * bounds.diagonal()));
    vwr->setCameraFocus( bounds.centre());    // Also resets the clipping range.
    fm->unlock();
}   // end resetCamera


void ActionResetCamera::doAction( Event)
{
    if ( MS::isViewSelected())
    {
        resetCamera( MS::selectedView());
        emit onEvent( Event::CAMERA_CHANGE);
    }   // end if
    else
    {
        for ( FMV* vwr : MS::viewers())
        {
            vwr->resetDefaultCamera( FaceTools::DEFAULT_CAMERA_DISTANCE);
            vwr->setCameraFocus( cv::Vec3f(0,0,0));
        }   // end for

        emit onEvent( {Event::CAMERA_CHANGE, Event::ALL_VIEWERS});
    }   // end else
}   // end doAction
