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

#include <ModelMoveInteractor.h>
#include <ModelMoveResponder.h>
#include <BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceView.h>
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Interactor::ModelMoveInteractor;
using FaceTools::Interactor::ModelMoveResponder;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControl;

ModelMoveInteractor::ModelMoveInteractor()
    : _moveModels(false), _moveResponder(nullptr), _affected(nullptr)
{
    _moveResponder = new ModelMoveResponder(this);
    connect( _moveResponder, &ModelMoveResponder::onAffineChange, this, &ModelViewerInteractor::onChangedData);
}   // end ctor


ModelMoveInteractor::~ModelMoveInteractor()
{
    delete _moveResponder;
}   // end dtor


// private
bool ModelMoveInteractor::setInteractionMode( const QPoint& p)
{
    QTools::InteractionMode imode = QTools::CAMERA_INTERACTION;
    _affected = testPoint(p);
    if ( movingModels() && _affected)
    {
        imode = QTools::ACTOR_INTERACTION;
        viewer()->setCursor( Qt::DragMoveCursor);
    }   // end if
    viewer()->setInteractionMode( imode);
    return false;
}   // end setInteractionMode


// private
FaceControl* ModelMoveInteractor::testPoint( const QPoint& p) const
{
    const vtkProp* prop = viewer()->getPointedAt(p);     // The prop pointed at
    if ( prop)
    {
        const FaceControlSet& fcs = qobject_cast<FaceModelViewer*>( viewer())->attached();
        for ( FaceControl* fc : fcs)
        {
            if ( fc->view()->isFace(prop))
                return fc;
        }   // end for
    }   // end if
    return nullptr;
}   // end testPoint


bool ModelMoveInteractor::leftButtonDown( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::middleButtonDown( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::rightButtonDown( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::mouseWheelForward( const QPoint& p) { return setInteractionMode(p);}
bool ModelMoveInteractor::mouseWheelBackward( const QPoint& p) { return setInteractionMode(p);}


void ModelMoveInteractor::actorStop()
{
    viewer()->setCursor( Qt::ArrowCursor);
    emit onActorStop(_affected);
}   // end actorStop

