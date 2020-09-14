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

#include <Interactor/MovementNotifier.h>
using FaceTools::Interactor::MovementNotifier;
using FaceTools::Action::Event;

void MovementNotifier::cameraStart()
{
    emit onEvent(Event::CAMERA_CHANGE);
}   // end cameraStart

void MovementNotifier::actorStart( const vtkProp3D*)
{
    emit onEvent(Event::ACTOR_MOVE);
}   // end actorStart
