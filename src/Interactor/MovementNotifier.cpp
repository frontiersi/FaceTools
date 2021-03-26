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

#include <Interactor/MovementNotifier.h>
#include <FaceModelViewer.h>
#include <ModelSelect.h>
#include <FaceModel.h>
#include <r3dvis/VtkTools.h>
using FaceTools::Interactor::MovementNotifier;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


void MovementNotifier::actorStart( const vtkProp3D* prop)
{
    FV* fv = viewFromActor( prop);
    if ( fv && fv == MS::selectedView())
        emit onActorStart(fv);
}   // end actorStart


void MovementNotifier::actorMove( const vtkProp3D* prop)
{
    // Propogate the matrix of the affected actor to all associated FaceView actors.
    FV* fv = viewFromActor( prop);
    if ( fv && fv == MS::selectedView())
    {
        const vtkMatrix4x4 *vt = fv->transformMatrix();
        for ( FV* f : fv->data()->fvs())
            f->pokeTransform( vt);
        MS::updateRender();
    }   // end if
}   // end actorMove


void MovementNotifier::actorStop( const vtkProp3D* prop)
{
    // Catch up the data transform to the actor's.
    FV* fv = viewFromActor( prop);
    if ( fv && fv == MS::selectedView())
    {
        FM *fm = fv->data();
        fm->lockForWrite();
        const Mat4f t = r3dvis::toEigen( fv->transformMatrix());
        fm->addTransformMatrix( t * fm->inverseTransformMatrix());
        fm->unlock();
        emit onActorStop(fv);
    }   // end if
}   // end actorStop


void MovementNotifier::cameraStart() { emit onCameraStart();}
void MovementNotifier::cameraMove() { emit onCameraMove();}
void MovementNotifier::cameraStop() { emit onCameraStop();}
