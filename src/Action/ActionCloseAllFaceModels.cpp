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

#include <Action/ActionCloseAllFaceModels.h>
#include <Action/FaceActionManager.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
#include <algorithm>
using FaceTools::Action::ActionCloseAllFaceModels;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::FAM;
using FaceTools::FMS;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::Action::ModelSelector;
using QMB = QMessageBox;


ActionCloseAllFaceModels::ActionCloseAllFaceModels( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks) {}

bool ActionCloseAllFaceModels::isAllowed( Event) { return FMM::numOpen() > 0;}

bool ActionCloseAllFaceModels::doBeforeAction( Event)
{
    bool doshowmsg = false;
    const FMS& models = FMM::opened();
    for ( FM* fm : models)
    {
        fm->lockForRead();
        if ( !fm->isSaved())
            doshowmsg = true;
        fm->unlock();
        if ( doshowmsg)
            break;
    }   // end for

    bool doclose = true;
    if ( doshowmsg)
    {
        static const QString msg = tr("Model(s) are unsaved! Really close?");
        doclose = QMB::Yes == QMB::warning( static_cast<QWidget*>(parent()), tr("Unsaved changes!"), msg, QMB::Yes | QMB::No, QMB::No);
    }   // end if

    return doclose;
}   // end doBeforeAction


void ActionCloseAllFaceModels::doAction( Event)
{
    UndoStates::clear();
    const FMS& models = FMM::opened();
    while ( !models.empty())
        FAM::close( *models.begin());
}   // end doAction


Event ActionCloseAllFaceModels::doAfterAction( Event)
{
    MS::setInteractionMode(IMode::CAMERA_INTERACTION);
    return Event::CLOSED_MODEL | Event::ALL_VIEWERS | Event::ALL_VIEWS | Event::VIEWER_CHANGE;
}   // end doAfterAction
