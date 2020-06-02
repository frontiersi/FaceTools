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

#include <Action/ActionEditPaths.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::PathsHandler;
using FaceTools::FM;
using FaceTools::Vis::FV;
using FaceTools::Vis::PathView;
using MS = FaceTools::Action::ModelSelector;

Q_DECLARE_METATYPE( FaceTools::Path)


ActionEditPaths::ActionEditPaths( const QString& dn, const QIcon& ic, PathsHandler::Ptr handler, const QKeySequence& ks)
    : ActionVisualise( dn, ic, &handler->visualisation(), ks), _handler( handler)
{
    connect( &*_handler, &PathsHandler::onStartedDrag, this, &ActionEditPaths::_doOnStartedDrag);
    connect( &*_handler, &PathsHandler::onFinishedDrag, [this](){ emit onEvent( Event::PATHS_CHANGE);});
}   // end ctor


bool ActionEditPaths::checkState( Event e)
{
    _handler->refreshState();
    return ActionVisualise::checkState(e);
}   // end checkState


Event ActionEditPaths::doAfterAction( Event e)
{
    MS::clearStatus();
    if ( isChecked())
        MS::showStatus( "Move path handles by left-clicking and dragging; right click to rename/delete.");
    return ActionVisualise::doAfterAction( e);
}   // end doAfterAction


void ActionEditPaths::saveState( UndoState &us) const
{
    const Path &path = us.model()->currentPaths().path(_pid);
    us.setName( "Move Path");
    us.setUserData( "Path", QVariant::fromValue( path));
}   // end saveState


void ActionEditPaths::restoreState( const UndoState& us)
{
    const Path &path = us.userData("Path").value<Path>();
    us.model()->currentAssessment()->paths().path(path.id()) = path;
}   // end restoreState


void ActionEditPaths::_doOnStartedDrag( PathView::Handle *h)
{
    _pid = h->pathId();
    storeUndo( this, Event::PATHS_CHANGE, false);
}   // end _doOnStartedDrag
