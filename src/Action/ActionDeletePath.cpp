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

#include <ActionDeletePath.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <ChangeEvents.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionDeletePath;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionDeletePath::ActionDeletePath( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _editor(nullptr)
{
}   // end ctor


bool ActionDeletePath::testEnabled() const
{
    bool enabled = false;
    FaceControl* fc = nullptr;
    assert(_editor);
    if ( readyCount() == 1 && _editor->isChecked())
    {
        PathSetInteractor* interactor = _editor->interactor();
        fc = interactor->hoverModel();
        enabled = isReady( fc) && interactor->hoverID() >= 0;
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionDeletePath::doAction( FaceControlSet& fcs)
{
    assert(_editor);
    PathSetInteractor* interactor = _editor->interactor();
    FaceControl* hc = fcs.first();
    assert(hc);
    fcs.clear();
    if ( interactor->deletePath())
        fcs.insert( hc);
    return !fcs.empty();
}   // end doAction


void ActionDeletePath::doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool)
{
    cs.insert( FaceTools::Action::METRICS_CHANGE);
}   // end doAfterAction
