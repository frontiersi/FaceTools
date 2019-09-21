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

#include <Interactor/ActorMoveNotifier.h>
#include <Action/ModelSelector.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Interactor::ActorMoveNotifier;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using MS = FaceTools::Action::ModelSelector;


void ActorMoveNotifier::actorStart( const vtkProp3D* prop)
{
    FV* fv = viewFromActor( prop);
    if ( fv)
    {
        MS::showStatus( "Moving model...");
        emit onActorStart(fv);
    }   // end if
}   // end actorStart


void ActorMoveNotifier::actorMove( const vtkProp3D* prop)
{
    FV* fv = viewFromActor( prop);
    if ( fv)
    {
        // Propogate the matrix of the affected actor to all of the associated FaceView actors.
        vtkMatrix4x4* mat = fv->actor()->GetMatrix();

        const FM* fm = fv->data();
        for ( FV* f : fm->fvs())
        {
            f->actor()->PokeMatrix( mat);
            f->syncVisualisationsToViewTransform();
            f->viewer()->updateRender();
        }   // end for
    }   // end if
}   // end actorMove


void ActorMoveNotifier::actorStop( const vtkProp3D* prop)
{
    FV* fv = viewFromActor( prop);
    if ( fv)
    {
        FM* fm = fv->data();
        fm->lockForWrite();
        // Catch up the data transform to the actor's.
        const cv::Matx44d& cmat = fm->model().transformMatrix();
        const cv::Matx44d vmat = RVTK::toCV( fv->actor()->GetMatrix());
        const cv::Matx44d dmat = vmat * cmat.inv();
        fm->addTransformMatrix( dmat);
        fm->unlock();
        MS::showStatus( "Finished moving model.", 5000);
        emit onActorStop(fv);
    }   // end if
}   // end actorStop
