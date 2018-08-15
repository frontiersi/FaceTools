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

#ifndef FACE_TOOLS_MODEL_MOVE_INTERACTOR_H
#define FACE_TOOLS_MODEL_MOVE_INTERACTOR_H

#include "ModelViewerInteractor.h"

namespace FaceTools {
namespace Interactor {
class ModelMoveResponder;

class FaceTools_EXPORT ModelMoveInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    ModelMoveInteractor();
    ~ModelMoveInteractor() override;

    // Call with true to make left and right button drags interact with the models rather than the camera.
    void setMoveModels( bool v) { _moveModels = v;}
    bool movingModels() const { return _moveModels;}

signals:
    // Notify of rotate, pan, and dolly events for the actor providing affected FaceControl.
    void onActorRotate( FaceControl*);
    void onActorPan( FaceControl*);
    void onActorDolly( FaceControl*);
    void onActorStop( FaceControl*);

    // Notify of rotate, pan, and dolly events for the camera.
    void onCameraRotate();
    void onCameraPan();
    void onCameraDolly();

private:
    bool _moveModels;
    ModelMoveResponder *_moveResponder;
    FaceControl *_affected;

    bool leftButtonDown( const QPoint&) override;
    bool middleButtonDown( const QPoint&) override;
    bool rightButtonDown( const QPoint&) override;
    bool mouseWheelForward( const QPoint&) override;
    bool mouseWheelBackward( const QPoint&) override;

    void cameraRotate() override { emit onCameraRotate();}
    void cameraDolly() override { emit onCameraDolly();}
    void cameraPan() override { emit onCameraPan();}

    void actorRotate() override { emit onActorRotate(_affected);}
    void actorDolly() override { emit onActorDolly(_affected);}
    void actorPan() override { emit onActorPan(_affected);}
    void actorStop() override;

    FaceControl* testPoint( const QPoint&) const;
    bool setInteractionMode( const QPoint&);
};  // end class

}   // end namespace
}   // end namespace

#endif
