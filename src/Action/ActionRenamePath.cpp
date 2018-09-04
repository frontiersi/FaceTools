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
#include <ActionEditPaths.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QInputDialog>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionRenamePath;
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::EventSet;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;


ActionRenamePath::ActionRenamePath( const QString& dn, const QIcon& ico, ActionEditPaths *e, QWidget *parent)
    : FaceAction(dn, ico), _editor(e), _parent(parent)
{
}   // end ctor


bool ActionRenamePath::testEnabled( const QPoint*) const
{
    bool enabled = false;
    assert(_editor);
    if ( _editor->isChecked() && gotReady())
    {
        const FV* fv = _editor->interactor()->hoverModel();
        enabled = fv && isReady(fv) && _editor->interactor()->hoverPathId() >= 0;
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionRenamePath::doAction( FVS& fvs, const QPoint&)
{
    assert(_editor);
    assert(fvs.size() == 1);
    FV* fv = fvs.first();
    assert(fv);
    assert(fv == _editor->interactor()->hoverModel());
    fvs.clear();

    FM* fm = fv->data();

    const int pid = _editor->interactor()->hoverPathId();
    assert(pid >= 0);

    //fm->lockForRead();
    QString clabel = fm->paths()->path(pid)->name.c_str();
    //fm->unlock();
    bool ok = false;
    QString nlabel = QInputDialog::getText( _parent, tr("Rename path"), tr("New path name:"), QLineEdit::Normal, clabel, &ok);
    if ( !ok)
        nlabel = "";

    if ( !nlabel.isEmpty() && nlabel != clabel)
    {
        //fm->lockForWrite();
        fm->paths()->path(pid)->name = nlabel.toStdString();
        fm->setSaved(false);
        fvs.insert(fm);
        _editor->interactor()->setCaptionInfo( fm, pid);
        //fm->unlock();
    }   // end if

    return true;
}   // end doAction
