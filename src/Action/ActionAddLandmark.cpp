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

#include <ActionAddLandmark.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <ChangeEvents.h>
#include <QInputDialog>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionAddLandmark;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionAddLandmark::ActionAddLandmark( const QString& dn, const QIcon& ico, QWidget *parent)
    : FaceAction( dn, ico), _editor(nullptr), _parent(parent)
{
}   // end ctor


bool ActionAddLandmark::testEnabled() const
{
    // Allow adding only if model being hovered over is same as selected
    // and no landmark is currently hovered over and editing is enabled.
    bool enabled = false;
    FaceControl* fc = nullptr;
    assert(_editor);
    if ( readyCount() == 1 && _editor->isChecked())
    {
        LandmarksInteractor* interactor = _editor->interactor();
        fc = interactor->hoverModel();
        enabled = isReady( fc) && interactor->hoverID() < 0;
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionAddLandmark::doAction( FaceControlSet& fcs)
{
    assert(_editor);
    LandmarksInteractor* interactor = _editor->interactor();
    QPoint p = interactor->viewer()->getMouseCoords();
    FaceControl* fc = fcs.first();
    assert(fc);
    fcs.clear();

    FaceModel *fm = fc->data();
    fm->lockForRead();

    // Get the name for the new landmark
    QString lname;
    while ( true)
    {
        bool ok = false;
        lname = QInputDialog::getText( _parent, tr("Landmark Name"), tr("New landmark name:"), QLineEdit::Normal, "", &ok);
        if ( !ok)
        {
            lname = "";
            break;
        }   // end if

        // If a landmark with the name already exists, show warning and try again.
        if ( fm->landmarks()->has(lname.toStdString()))
        {
            static const QString msg = tr("A landmark with that name already exists! ") +
                                       tr("Choose a different name or move the existing landmark into a different position.");
            QMessageBox::information( _parent, tr("Landmark exists!"), msg, QMessageBox::Ok);
        }   // end if
        else
            break;
    }   // end while

    fm->unlock();

    int id = -1;
    if ( !lname.isEmpty())
    {
        id = interactor->addLandmark( lname.toStdString());
        if ( id >= 0)
            fcs.insert( fc);
    }   // end if

    return id >= 0;
}   // end doAction


void ActionAddLandmark::doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool)
{
    cs.insert( LANDMARKS_CHANGE);
}   // end doAfterAction
