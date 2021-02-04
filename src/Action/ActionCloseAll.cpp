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

#include <Action/ActionCloseAll.h>
#include <Action/FaceActionManager.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
using FaceTools::Action::ActionCloseAll;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::FAM;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionCloseAll::ActionCloseAll( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks)
{
    addRefreshEvent( Event::LOADED_MODEL | Event::CLOSED_MODEL);
}   // end ctor

bool ActionCloseAll::isAllowed( Event) { return FMM::numOpen() > 1;}

bool ActionCloseAll::doBeforeAction( Event)
{
    bool doshowmsg = false;
    for ( FM* fm : FMM::opened())
    {
        FM::RPtr fmptr = fm->scopedReadLock();
        if ( !fm->isSaved())
        {
            doshowmsg = true;
            break;
        }   // end if
    }   // end for

    bool doclose = true;
    if ( doshowmsg)
    {
        static const QString msg = tr("Model(s) are not saved! Really close?");
        doclose = QMB::Yes == QMB::warning( static_cast<QWidget*>(parent()),
                tr("Unsaved Changes!"), QString("<p align='center'>%1</p>").arg(msg), QMB::Yes | QMB::No, QMB::No);
    }   // end if

    return doclose;
}   // end doBeforeAction


void ActionCloseAll::doAction( Event)
{
    UndoStates::clear();
    while ( !FMM::opened().empty())
        FAM::close( *FMM::opened().begin());
}   // end doAction


Event ActionCloseAll::doAfterAction( Event)
{
    MS::setInteractionMode(IMode::CAMERA_INTERACTION);
    return Event::CLOSED_MODEL | Event::ALL_VIEWERS | Event::ALL_VIEWS | Event::VIEWER_CHANGE;
}   // end doAfterAction
