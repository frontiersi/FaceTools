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
using FaceTools::Action::ChangeEventSet;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionAddPath::ActionAddPath( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _editor(nullptr)
{
}   // end ctor


bool ActionAddPath::testEnabled() const
{
    // Allow path adding only if the model being hovered over is the same as
    // the selected model AND there's no handle currently being hovered over
    // and path editing is currently enabled.
    bool enabled = false;
    FaceControl* fc = nullptr;
    assert(_editor);
    if ( readyCount() == 1 && _editor->isChecked())
    {
        PathSetInteractor* interactor = _editor->interactor();
        fc = interactor->hoverModel();
        enabled = isReady( fc) && interactor->hoverID() < 0;
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionAddPath::doAction( FaceControlSet& fcs)
{
    assert(_editor);
    PathSetInteractor* interactor = _editor->interactor();
    QPoint p = interactor->viewer()->getMouseCoords();
    FaceControl* hc = fcs.first();
    assert(hc);
    fcs.clear();
    int pid = interactor->addPath();
    if ( pid >= 0)
    {
        fcs.insert( hc);
        interactor->setDrag( pid, p);
    }   // end if
    return pid >= 0;
}   // end doAction


void ActionAddPath::doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool)
{
    cs.insert( METRICS_CHANGE);
}   // end doAfterAction
