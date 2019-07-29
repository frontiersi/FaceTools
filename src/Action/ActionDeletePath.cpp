/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ActionDeletePath.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionDeletePath;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::PathsInteractor;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionDeletePath::ActionDeletePath( const QString& dn, const QIcon& ico,
                              PathSetVisualisation *vis,
                              PathsInteractor::Ptr pint)
    : FaceAction( dn, ico), _vis(vis), _pint(pint)
{
    connect( &*_pint, &PathsInteractor::onLeavePath, this, &ActionDeletePath::doOnLeavePath);
    connect( &*_pint, &PathsInteractor::onEnterPath, this, &ActionDeletePath::doOnEnterPath);
}   // end ctor


bool ActionDeletePath::checkEnable( Event)
{
    const FV* fv = _pint->view();
    if ( MS::interactionMode() == IMode::ACTOR_INTERACTION || !fv)
        return false;
    return fv == MS::selectedView() && _pint->hoverPath();
}   // end checkEnabled


void ActionDeletePath::doAction( Event)
{
    storeUndo(this, Event::PATHS_CHANGE);
    FV* fv = MS::selectedView();
    FM* fm = fv->data();
    const int pid = _pint->hoverPath()->pathId();
    assert(pid >= 0);
    fm->lockForWrite();
    fm->removePath(pid);
    fm->unlock();
    _pint->leavePath();
    emit onEvent( Event::PATHS_CHANGE);
}   // end doAction


void ActionDeletePath::doOnLeavePath()
{
    setLocked(true);
    refreshState();
}   // end doOnLeavePath


void ActionDeletePath::doOnEnterPath()
{
    setLocked(false);
    refreshState();
}   // end doOnEnterPath

