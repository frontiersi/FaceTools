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

#include <ActionRenamePath.h>
#include <ActionEditPaths.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QInputDialog>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionRenamePath;
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::ChangeEventSet;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::Vis::PathSetVisualisation;


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
        PathSetInteractor* interactor = _editor->interactor();
        const FaceControl* fc = interactor->hoverModel();
        enabled = fc && isReady(fc) && interactor->hoverID() >= 0;
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionRenamePath::doAction( FaceControlSet& fcs, const QPoint&)
{
    assert(fcs.size() == 1);
    assert(_editor);
    FaceControl* fc = fcs.first();
    fcs.clear();
    assert(fc);
    PathSetInteractor* interactor = _editor->interactor();
    assert(fc == interactor->hoverModel());
    int pathID = interactor->hoverID();
    assert(pathID >= 0);

    FaceModel* fm = fc->data();
    fm->lockForRead();
    QString clabel = fm->paths()->path(pathID)->name.c_str();
    fm->unlock();

    bool ok = false;
    QString nlabel = QInputDialog::getText( _parent, tr("Rename path"), tr("New path name:"), QLineEdit::Normal, clabel, &ok);
    if ( !ok)
        nlabel = "";

    if ( !nlabel.isEmpty() && nlabel != clabel)
    {
        fm->lockForWrite();
        fm->paths()->path(pathID)->name = nlabel.toStdString();
        fm->setSaved(false);
        fm->unlock();
        fcs.insert(fc);
        qobject_cast<PathSetVisualisation*>( _editor->visualisation())->setCaptions( fc, pathID);
    }   // end if

    return true;
}   // end doAction
