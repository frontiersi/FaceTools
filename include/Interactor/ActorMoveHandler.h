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

#ifndef FACE_TOOLS_INTERACTOR_ACTOR_MOVE_HANDLER_H
#define FACE_TOOLS_INTERACTOR_ACTOR_MOVE_HANDLER_H

#include <ModelViewerInteractor.h>
#include <FaceTypes.h>

namespace FaceTools { namespace Interactor {

class ActorMoveHandler : public Interactor::MVI
{ Q_OBJECT
signals:
    void onActorStart( Vis::FV*);
    void onActorStop( Vis::FV*);

protected:
    void actorStart( const vtkProp3D*) override;
    void actorMove( const vtkProp3D*) override;
    void actorStop( const vtkProp3D*) override;
};  // end class

}}   // end namespace

#endif
