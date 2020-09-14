/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Interactor/ActorMoveNotifier.h>
#include <Action/ModelSelector.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <r3dvis/VtkTools.h>
using FaceTools::Interactor::ActorMoveNotifier;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


bool ActorMoveNotifier::_isValidView( const vtkProp3D *prop) const
{
    FV *fv = viewFromActor(prop);
    return fv && fv == MS::selectedView();
}   // end _isValidView


void ActorMoveNotifier::actorStart( const vtkProp3D* prop)
{
    if ( _isValidView(prop))
    {
        FV* fv = viewFromActor( prop);
        emit onActorStart(fv);
        _cam = fv->viewer()->camera();
    }   // end if
}   // end actorStart


void ActorMoveNotifier::actorMove( const vtkProp3D* prop)
{
    if ( _isValidView(prop)) // Propogate the matrix of the affected actor to all associated FaceView actors.
    {
        FV* fv = viewFromActor( prop);
        const vtkMatrix4x4 *vt = fv->transformMatrix();
        for ( FV* f : fv->data()->fvs())
            f->pokeTransform( vt);

        /*
        // Also transform the camera focus and position with respect to the model.
        // This is also important for ensuring that scaling actors resize correctly.
        const Mat4f& cmat = fv->data()->mesh().transformMatrix();
        const Mat4f T = r3dvis::toEigen( vt) * cmat.inverse();
        const r3d::CameraParams cam(  r3d::transform( T, _cam.pos()),
                                      r3d::transform( T, _cam.focus()),
                                      T.block<3,3>(0,0) * _cam.up(),
                                      _cam.fov());
        fv->viewer()->setCamera( cam);
        */

        MS::updateRender();
    }   // end if
}   // end actorMove


void ActorMoveNotifier::actorStop( const vtkProp3D* prop)
{
    if ( _isValidView(prop))
    {
        FV* fv = viewFromActor( prop);
        // Catch up the data transform to the actor's.
        const vtkMatrix4x4 *vt = fv->transformMatrix();
        FM* fm = fv->data();
        fm->lockForWrite();
        const Mat4f& imat = fm->inverseTransformMatrix();
        const Mat4f t = r3dvis::toEigen( vt);
        fm->addTransformMatrix( t * imat);
        fm->unlock();
        emit onActorStop(fv);
    }   // end if
}   // end actorStop


void ActorMoveNotifier::cameraStop() { emit onCameraStop();}
