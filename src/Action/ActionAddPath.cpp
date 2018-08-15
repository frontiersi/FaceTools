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

#include <ActionAddPath.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <ChangeEvents.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionAddPath;
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionAddPath::ActionAddPath( const QString& dn, const QIcon& ico, ActionEditPaths* e)
    : FaceAction( dn, ico), _editor(e)
{
}   // end ctor


bool ActionAddPath::testEnabled( const QPoint*) const
{
    // Allow path adding only if the model being hovered over is the same as
    // the selected model AND there's no handle currently being hovered over
    // and path editing is currently enabled.
    PathSetInteractor* interactor = _editor->interactor();
    const FaceControl* fc = interactor->hoverModel();
    return isReady( fc) && interactor->hoverID() < 0;
}   // end testEnabled


bool ActionAddPath::doBeforeAction( FaceControlSet&, const QPoint&)
{
    if ( !_editor->isChecked())
        _editor->process( ready(), true);   // Flip the visualisation on if not already
    return true;
}   // end doBeforeAction


bool ActionAddPath::doAction( FaceControlSet& fcs, const QPoint& p)
{
    assert(_editor);
    PathSetInteractor* interactor = _editor->interactor();
    fcs.clear();
    int pid = interactor->addPath(p);
    if ( pid >= 0)
        fcs.insert( interactor->hoverModel()->data());
    return pid >= 0;
}   // end doAction
