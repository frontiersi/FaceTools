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

#include <Action/ActionSetFocus.h>
#include <Interactor/LandmarksHandler.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
using FaceTools::Action::ActionSetFocus;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::ModelSelect;


ActionSetFocus::ActionSetFocus( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks) {}


bool ActionSetFocus::isAllowed( Event) { return MS::isViewSelected();}


bool ActionSetFocus::doBeforeAction( Event)
{
    const Vis::FV *fv = MS::selectedView();
    QPoint mp = primedMousePos();
    if ( mp.x() < 0)
        mp = fv->viewer()->mouseCoords();
    // If point projects to surface, check if overlaps with landmarks
    const bool go = fv->projectToSurface( mp, _vproj);
    if ( go && MS::handler<Interactor::LandmarksHandler>()->visualisation().isVisible(fv))
    {
        const float srng = fv->viewer()->snapRange();
        _vproj = fv->data()->currentLandmarks().snapTo( _vproj, srng*srng);
    }   // end if
    return go;
}   // end doBeforeAction


void ActionSetFocus::doAction( Event)
{
    r3d::CameraParams cam = MS::selectedViewer()->camera();
    cam.set( _vproj, _vproj + cam.pos() - cam.focus());
    MS::selectedViewer()->setCamera(cam);
}   // end doAction


Event ActionSetFocus::doAfterAction( Event) { return Event::CAMERA_CHANGE;}
