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
#include <Interactor/PathsHandler.h>
#include <FaceModel.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::ActionAddPath;
using FaceTools::Interactor::PathsHandler;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


ActionAddPath::ActionAddPath( const QString& dn, const QIcon& ico, const QKeySequence &ks)
    : FaceAction( dn, ico, ks)
{
    const PathsHandler *h = MS::handler<PathsHandler>();
    connect( h, &PathsHandler::onEnterHandle, [this](){ this->refresh();});
    connect( h, &PathsHandler::onLeaveHandle, [this](){ this->refresh();});
}   // end ctor


// Note always allowed but doBeforeAction will cancel if the mouse position does
// not project to a point on the currently selected model.
bool ActionAddPath::isAllowed( Event) { return MS::isViewSelected() && MS::interactionMode() == IMode::CAMERA_INTERACTION;}


bool ActionAddPath::doBeforeAction( Event)
{
    QPoint mp = primedMousePos();
    if ( mp.x() < 0)
        mp = MS::selectedViewer()->mouseCoords();
    return MS::selectedView()->projectToSurface( mp, _vproj);
}   // end doBeforeAction


void ActionAddPath::doAction( Event)
{
    PathsHandler *handler = MS::handler<PathsHandler>();
    const Vis::PathView::Handle *h = handler->hoverPath();
    if ( h)
    {
        _vproj = h->viewPos();
        handler->endDragging();    // It's possible we might be in the middle of adding a path.
        handler->leavePath();
    }   // end if

    storeUndo( this, Event::PATHS_CHANGE);
    const int pid = MS::selectedModel()->addPath( _vproj);
    handler->addPath( pid);
}   // end doAction


Event ActionAddPath::doAfterAction( Event) { return Event::PATHS_CHANGE;}
