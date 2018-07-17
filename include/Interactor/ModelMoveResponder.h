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

#ifndef FACE_TOOLS_MODEL_MOVE_RESPONDER_H
#define FACE_TOOLS_MODEL_MOVE_RESPONDER_H

/**
 * Takes adjusted view transforms from user interactions of FaceControls attended
 * to by ModelMoveInteractor and coordinates the movement of other FaceControls
 * attached to the model as well as associated view actors such as landmarks etc.
 * On finish of a movement, all data are updated on the model.
 */

#include "ModelMoveInteractor.h"

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT ModelMoveResponder : public QObject
{ Q_OBJECT
public:
    explicit ModelMoveResponder( ModelMoveInteractor*);

signals:
    // Fired after finalising transform on the FaceModel through the given FaceControl.
    void onAffineChange( const FaceControl*);

public slots:
    // Update data using the actor transform matrix.
    void doOnActorFinishPos( const FaceControl*);

private slots:
    void doOnActorChangedPos( const FaceControl*);

private:
    ModelMoveResponder( const ModelMoveResponder&) = delete;
    void operator=( const ModelMoveResponder&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
