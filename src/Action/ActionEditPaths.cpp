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

#include <ActionEditPaths.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::Event;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::PathsInteractor;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::FM;
using FaceTools::Vis::FV;
using FaceTools::Vis::PathView;
using MS = FaceTools::Action::ModelSelector;



ActionEditPaths::ActionEditPaths( const QString& dn, const QIcon& ic, PathSetVisualisation* vis, PathsInteractor::Ptr pint, const QKeySequence& ks)
    : ActionVisualise( dn, ic, vis, ks), _pint( pint), _vis(vis)
{
    connect( &*_pint, &PathsInteractor::onUpdated, [this](){ emit onEvent( Event::PATHS_CHANGE);});
    addTriggerEvent( Event::PATHS_CHANGE);
}   // end ctor


bool ActionEditPaths::checkState( Event e)
{
    const bool chk = ActionVisualise::checkState(e);
    _pint->setEnabled(chk);
    if ( chk)
        _vis->refresh( MS::selectedModel());
    return chk;
}   // end checkState


void ActionEditPaths::doAction( Event e)
{
    ActionVisualise::doAction(e);
    if ( isChecked())
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        MS::showStatus( "Move path handles by left-clicking and dragging; right click to rename/delete.");
        if ( _pint->hoverPath())  // Ensure captions of hovered over path is up-to-date.
            _pint->setCaption( MS::selectedView(), _pint->hoverPath()->pathId());
    }   // end if
    else
        MS::clearStatus();
}   // end doAction
