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

#include <Action/ActionAddPath.h>
#include <FaceModel.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::ActionAddPath;
using FaceTools::Interactor::PathsHandler;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


ActionAddPath::ActionAddPath( const QString& dn, const QIcon& ico, PathsHandler::Ptr handler)
    : FaceAction( dn, ico), _handler(handler)
{
}   // end ctor


bool ActionAddPath::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    return fv && MS::interactionMode() != IMode::ACTOR_INTERACTION
              && fv->isPointOnFace( primedMousePos())
              && _handler->hoverPath() == nullptr;
}   // end isAllowed


void ActionAddPath::doAction( Event)
{
    storeUndo( this, Event::PATHS_CHANGE);
    const FV *fv = MS::selectedView();
    Vec3f pos = Vec3f::Zero();
    fv->projectToSurface( primedMousePos(), pos);
    FM* fm = fv->data();
    fm->lockForWrite();
    const int pid = fm->addPath( pos);
    fm->unlock();
    _handler->addPath( fv, pid);
}   // end doAction


Event ActionAddPath::doAfterAction( Event) { return Event::PATHS_CHANGE | Event::VIEW_CHANGE;}
