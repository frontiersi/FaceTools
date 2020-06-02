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

#include <Action/ActionRenamePath.h>
#include <FaceModel.h>
#include <QInputDialog>
#include <cassert>
using FaceTools::Action::ActionRenamePath;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::PathsHandler;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionRenamePath::ActionRenamePath( const QString& dn, const QIcon& ico, PathsHandler::Ptr handler)
    : FaceAction(dn, ico), _handler(handler)
{
}   // end ctor


bool ActionRenamePath::isAllowed( Event)
{
    return MS::interactionMode() == IMode::CAMERA_INTERACTION && _handler->hoverPath();
}   // end isAllowed


void ActionRenamePath::doAction( Event)
{
    storeUndo(this, Event::PATHS_CHANGE);
    FM* fm = MS::selectedModel();
    const int pid = _handler->hoverPath()->pathId();
    assert(pid >= 0);

    fm->lockForRead();
    QString clabel = fm->currentAssessment()->paths().name(pid);
    fm->unlock();

    QWidget* prnt = static_cast<QWidget*>(parent());
    bool ok = false;
    const QString nlabel = QInputDialog::getText( prnt, tr("Rename path"), tr("New path name:"), QLineEdit::Normal, clabel, &ok);

    _e = Event::NONE;
    if ( ok && nlabel != clabel)
    {
        fm->lockForWrite();
        fm->renamePath( pid, nlabel);
        fm->unlock();
        _e = Event::PATHS_CHANGE;
    }   // end if
}   // end doAction


Event ActionRenamePath::doAfterAction( Event) { return _e;}
