/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionClose.h>
#include <Action/FaceActionManager.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
#include <QFileInfo>
using FaceTools::Action::ActionClose;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::FAM;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionClose::ActionClose( const QString& dname, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dname, ico, ks)
{
    addRefreshEvent( Event::LOADED_MODEL | Event::CLOSED_MODEL);
}   // end ctor

bool ActionClose::isAllowed( Event) { return MS::isViewSelected();}


bool ActionClose::doBeforeAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    bool inPreferredFormat = FMM::hasPreferredFileFormat( *fm);

    bool doclose = false;
    if ( fm->isSaved())
        doclose = true;
    else
    {
        const QString fname = QFileInfo( FMM::filepath(*fm)).fileName();
        QString msg = tr( ("Model '" + fname.toStdString() + "' is unsaved! Really close?").c_str());
        if ( fm->hasMetaData() && !inPreferredFormat)
            msg = tr("Not saved as 3DF; data will be lost! Really close?");
        doclose = QMB::Yes == QMB::warning( static_cast<QWidget*>(parent()),
                tr("Unsaved Changes!"), QString("<p align='center'>%1</p>").arg(msg), QMB::Yes | QMB::No, QMB::No);
    }   // end if

    return doclose;
}   // end doBeforeAction


void ActionClose::doAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    FV *fv = MS::selectedView();
    UndoStates::clear( fv->data());
    fv = FAM::close( fv->data());
    _ev = Event::CLOSED_MODEL | Event::ALL_VIEWERS;
    if ( fv)
    {
        MS::setSelected(fv);
        _ev |= Event::MODEL_SELECT;
    }   // end if
}   // end doAction


Event ActionClose::doAfterAction( Event) { return _ev;}
