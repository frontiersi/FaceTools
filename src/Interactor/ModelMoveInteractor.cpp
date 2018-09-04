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

#include <ModelMoveInteractor.h>
#include <BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Interactor::ModelMoveInteractor;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::FM;

ModelMoveInteractor::ModelMoveInteractor() : _moveModels(false), _afv(nullptr) {}


bool ModelMoveInteractor::leftButtonDown( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::middleButtonDown( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::rightButtonDown( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::mouseWheelForward( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::mouseWheelBackward( const QPoint& p) { return setInteractionMode(p);}


// private
bool ModelMoveInteractor::setInteractionMode( const QPoint& p)
{
    QTools::InteractionMode imode = QTools::CAMERA_INTERACTION;
    setAffected(p);
    if ( movingModels() && _afv)
    {
        imode = QTools::ACTOR_INTERACTION;
        viewer()->setCursor( Qt::DragMoveCursor);
    }   // end if
    viewer()->setInteractionMode( imode);
    return false;
}   // end setInteractionMode


// private
void ModelMoveInteractor::setAffected( const QPoint& p)
{
    _afv = nullptr;
    const vtkProp* prop = viewer()->getPointedAt(p);     // The prop pointed at
    if ( prop)
    {
        const FVS& fvs = qobject_cast<FMV*>( viewer())->attached();
        for ( FV* fv : fvs)
        {
            if ( fv->actor() == prop)
            {
                _afv = fv;
                return;
            }   // end if
        }   // end for
    }   // end if
}   // end setAffected


// private
void ModelMoveInteractor::pokeTransform()
{
    assert(_afv);
    vtkMatrix4x4* m = _afv->actor()->GetMatrix();   // The moved actor's transform to be poked everywhere. LOL.
    _afv->data()->pokeTransform( m); // Poke everything.
    _afv->data()->updateRenderers();
}   // end pokeTransform


// private
void ModelMoveInteractor::actorStop()
{
    viewer()->setCursor( Qt::ArrowCursor);
    assert(_afv);
    FM* fm = _afv->data();
    fm->lockForWrite();
    fm->transform( RVTK::toCV( _afv->actor()->GetMatrix()));
    fm->unlock();
    emit onChangedData( _afv);
}   // end actorStop
