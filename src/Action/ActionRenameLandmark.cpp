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

#include <ActionRenameLandmark.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QInputDialog>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionRenameLandmark;
using FaceTools::Action::ChangeEventSet;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::Vis::LandmarksVisualisation;


ActionRenameLandmark::ActionRenameLandmark( const QString& dn, const QIcon& ico, QWidget *parent)
    : FaceAction(dn, ico), _editor(nullptr), _parent(parent)
{
}   // end ctor


bool ActionRenameLandmark::testEnabled() const
{
    bool enabled = false;
    assert(_editor);
    if ( _editor->isChecked() && readyCount() == 1)
    {
        LandmarksInteractor* interactor = _editor->interactor();
        FaceControl* fc = interactor->hoverModel();
        enabled = fc && isReady(fc) && interactor->hoverID() >= 0;
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionRenameLandmark::doAction( FaceControlSet& fcs)
{
    assert(fcs.size() == 1);
    assert(_editor);
    FaceControl* fc = fcs.first();
    fcs.clear();
    assert(fc);
    LandmarksInteractor* interactor = _editor->interactor();
    assert(fc == interactor->hoverModel());
    int id = interactor->hoverID();
    assert(id >= 0);

    FaceModel* fm = fc->data();
    fm->lockForRead();
    QString clabel = fm->landmarks()->get(id)->name.c_str();

    QString nlabel;
    while ( true)
    {
        bool ok = false;
        nlabel = QInputDialog::getText( _parent, tr("Rename landmark"), tr("New landmark name:"), QLineEdit::Normal, clabel, &ok);
        if ( !ok)
        {
            nlabel = "";
            break;
        }   // end if

        // If label is the same as some other landmark - and not the current label, advise the user to choose something different.
        if ( fm->landmarks()->has(nlabel.toStdString()) && nlabel != clabel)
        {
            static const QString msg = tr("A different landmark with that name already exists! ") +
                                        tr("Landmark labels must be unique - use something else.");
            QMessageBox::information( _parent, tr("Non-unique name!"), msg, QMessageBox::Ok);
        }   // end if
        else
            break;
    }   // end while

    fm->unlock();

    if ( !nlabel.isEmpty())
    {
        fm->lockForWrite();
        fm->landmarks()->get(id)->name = nlabel.toStdString();
        fm->setSaved(false);
        fm->unlock();
        fcs.insert(fc);
        qobject_cast<LandmarksVisualisation*>( _editor->visualisation())->refreshLandmark( fc, id);
    }   // end if

    return true;
}   // end doAction


void ActionRenameLandmark::doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool)
{ 
    cs.insert(LANDMARKS_CHANGE);
}   // end doAfterAction
