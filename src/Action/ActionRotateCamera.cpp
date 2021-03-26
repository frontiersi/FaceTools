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

#include <Action/ActionRotateCamera.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <r3d/CameraParams.h>
using FaceTools::Action::ActionRotateCamera;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionRotateCamera::ActionRotateCamera( const QString& dn, const QKeySequence& ks, float v, float h)
    : FaceAction( dn, QIcon(), ks), _vdegs(v), _hdegs(h) {}


bool ActionRotateCamera::isAllowed( Event) { return MS::isViewSelected();}


bool ActionRotateCamera::doBeforeAction( Event e) { return isAllowed( e);}


void ActionRotateCamera::doAction( Event)
{
    FV *fv = MS::selectedView();
    assert(fv);
    r3d::CameraParams cp = fv->viewer()->camera();
    if ( _vdegs != 0.0f)
        cp.tilt( _vdegs);
    if ( _hdegs != 0.0f)
        cp.rotate( _hdegs);
    fv->viewer()->setCamera( cp);
}   // end doAction


Event ActionRotateCamera::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return Event::CAMERA_CHANGE;
}   // end doAfterAction
