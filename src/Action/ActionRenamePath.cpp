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

#include <ActionRenamePath.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QInputDialog>
#include <cassert>
using FaceTools::Action::ActionRenamePath;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::PathsInteractor;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionRenamePath::ActionRenamePath( const QString& dn, const QIcon& ico, PathsInteractor::Ptr pint)
    : FaceAction(dn, ico), _pint(pint)
{
    connect( &*_pint, &PathsInteractor::onLeavePath, this, &ActionRenamePath::doOnLeavePath);
    connect( &*_pint, &PathsInteractor::onEnterPath, this, &ActionRenamePath::doOnEnterPath);
}   // end ctor


bool ActionRenamePath::checkEnable( Event)
{
    const FV* fv = _pint->view();
    return fv == MS::selectedView() && _pint->hoverPath();
}   // end checkEnabled


void ActionRenamePath::doAction( Event)
{
    storeUndo(this, Event::PATHS_CHANGE);
    const FV* fv = MS::selectedView();
    FM* fm = fv->data();
    const int pid = _pint->hoverPath()->pathId();
    assert(pid >= 0);

    fm->lockForRead();
    QString clabel = fm->paths().path(pid)->name.c_str();
    fm->unlock();

    QWidget* prnt = static_cast<QWidget*>(parent());
    bool ok = false;
    QString nlabel = QInputDialog::getText( prnt, tr("Rename path"), tr("New path name:"), QLineEdit::Normal, clabel, &ok);
    if ( !ok)
        nlabel = "";

    if ( !nlabel.isEmpty() && nlabel != clabel)
    {
        fm->lockForWrite();
        fm->renamePath( pid, nlabel);
        fm->unlock();
        emit onEvent( Event::PATHS_CHANGE);
    }   // end if
}   // end doAction


void ActionRenamePath::doOnLeavePath()
{
    setLocked(true);
    refreshState();
}   // end doOnLeavePath


void ActionRenamePath::doOnEnterPath()
{
    setLocked(false);
    refreshState();
}   // end doOnEnterPath

