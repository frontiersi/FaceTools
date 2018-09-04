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

#include <ActionAddLandmark.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <FaceTools.h>
#include <QInputDialog>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionAddLandmark;
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::EventSet;
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;


ActionAddLandmark::ActionAddLandmark( const QString& dn, const QIcon& ico, ActionEditLandmarks* e, QWidget *parent)
    : FaceAction( dn, ico), _editor(e), _parent(parent)
{
}   // end ctor


bool ActionAddLandmark::testEnabled( const QPoint*) const
{
    const bool g = _editor->interactor()->hoverId() < 0;
    const FV* fv = _editor->interactor()->hoverModel();
    return isReady( fv) && g;
}   // end testEnabled


bool ActionAddLandmark::doAction( FVS& fvs, const QPoint& p)
{
    assert(_editor);
    FV* fv = fvs.first();
    assert(fv);
    fvs.clear();

    FM* fm = fv->data();
    //fm->lockForWrite();

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

    int id = -1;
    if ( !lname.isEmpty())
    {
        cv::Vec3f hpos; // Get landmark position by projecting to model surface
        if ( fv->projectToSurface(p, hpos))
        {
            id = fm->landmarks()->set( lname.toStdString(), hpos);
            fm->setSaved(false);
            fvs.insert( fm);
        }   // end if
    }   // end if

    //fm->unlock();

    return id >= 0;
}   // end doAction
