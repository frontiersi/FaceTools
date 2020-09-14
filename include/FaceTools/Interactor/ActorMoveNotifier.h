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

#ifndef FACE_TOOLS_INTERACTOR_ACTOR_MOVE_NOTIFIER_H
#define FACE_TOOLS_INTERACTOR_ACTOR_MOVE_NOTIFIER_H

#include "ViewerNotifier.h"
#include <r3d/CameraParams.h>

namespace FaceTools { namespace Interactor {

class ActorMoveNotifier : public QObject, public ViewerNotifier
{ Q_OBJECT
signals:
    void onActorStart( Vis::FV*);
    void onActorStop( Vis::FV*);
    void onCameraStop();

protected:
    void cameraStop() override;
    void actorStart( const vtkProp3D*) override;
    void actorMove( const vtkProp3D*) override;
    void actorStop( const vtkProp3D*) override;

private:
    r3d::CameraParams _cam;
    bool _isValidView( const vtkProp3D*) const;
};  // end class

}}   // end namespace

#endif
