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

#include <Action/ActionAddPath.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
#include <FaceTools.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::ActionAddPath;
using FaceTools::Interactor::PathsHandler;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionAddPath::ActionAddPath( const QString& dn, const QIcon& ico, PathsHandler::Ptr handler)
    : FaceAction( dn, ico), _handler(handler)
{
    connect( &*_handler, &PathsHandler::onLeavePath, this, &ActionAddPath::doOnLeavePath);
    connect( &*_handler, &PathsHandler::onEnterPath, this, &ActionAddPath::doOnEnterPath);
}   // end ctor


bool ActionAddPath::checkEnable( Event)
{
    const FV* fv = MS::selectedView();
    if ( MS::interactionMode() == IMode::ACTOR_INTERACTION || !fv)
        return false;
    return fv->isPointOnFace( primedMousePos()) && _handler->hoverPath() == nullptr;
}   // end checkEnabled


void ActionAddPath::doAction( Event)
{
    storeUndo( this, Event::PATHS_CHANGE);

    FV* fv = MS::selectedView();
    cv::Vec3f hpos;
    if ( fv->projectToSurface( primedMousePos(), hpos))
    {
        _handler->setEnabled(true);
        _handler->addPath( fv, hpos);
        emit onEvent( Event::PATHS_CHANGE);
    }   // end if
}   // end doAction


void ActionAddPath::doOnLeavePath()
{
    setLocked(false);
    refreshState();
}   // end doOnLeavePath


void ActionAddPath::doOnEnterPath()
{
    setLocked(true);
    refreshState();
}   // end doOnEnterPath
