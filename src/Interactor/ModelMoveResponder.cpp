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

#include <ModelMoveResponder.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <VtkTools.h>
using FaceTools::Interactor::ModelMoveInteractor;
using FaceTools::Interactor::ModelMoveResponder;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ModelMoveResponder::ModelMoveResponder( ModelMoveInteractor* mmi)
{
    connect( mmi, &ModelMoveInteractor::onActorRotate, this, &ModelMoveResponder::doOnActorChangedPos);
    connect( mmi, &ModelMoveInteractor::onActorDolly,  this, &ModelMoveResponder::doOnActorChangedPos);
    connect( mmi, &ModelMoveInteractor::onActorPan,    this, &ModelMoveResponder::doOnActorChangedPos);
    connect( mmi, &ModelMoveInteractor::onActorStop,   this, &ModelMoveResponder::doOnActorFinishPos);
}   // end ctor


// public
void ModelMoveResponder::doOnActorFinishPos( const FaceControl* fc)
{
    FaceModel* fm = fc->data();
    fm->lockForWrite();
    fm->transform( RVTK::toCV( fc->view()->userTransform()));
    fm->unlock();
    emit onAffineChange(fc);
}   // end doOnActorFinishPos


// private
void ModelMoveResponder::doOnActorChangedPos( const FaceControl* fc)
{
    FaceModel* fm = fc->data();
    vtkSmartPointer<vtkMatrix4x4> M = fc->view()->userTransform();  // The user transform (adjustment from I)
    //static int i = 0;
    //std::cerr << "  T " << (i++) << ") ------------------------- " << std::endl;
    //RVTK::print( std::cerr, M);
    vtkMatrix4x4* m = M.GetPointer();
    for ( const FaceControl* f : fm->faceControls())
    {
        f->view()->pokeTransform( m, fc != f);  // Transform other visualisation layers on this actor
        //M = fc->view()->userTransform();
        //RVTK::print( std::cerr, M);
    }   // end fo
    fm->updateRenderers();
}   // end doOnActorChangedPos
