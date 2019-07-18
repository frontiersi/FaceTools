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

#include <ActionCloseAllFaceModels.h>
#include <FaceActionManager.h>
#include <FaceModelManager.h>
#include <QMessageBox>
#include <algorithm>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCloseAllFaceModels;
using FaceTools::Action::Event;
using FaceTools::FileIO::FMM;
using FaceTools::Action::FAM;
using FaceTools::FMS;
using FaceTools::FM;
using FaceTools::Action::EventGroup;
using MS = FaceTools::Action::ModelSelector;


ActionCloseAllFaceModels::ActionCloseAllFaceModels( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks) {}

bool ActionCloseAllFaceModels::checkEnable( Event) { return FMM::numOpen() > 0;}

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
        static const QString msg = tr("Model(s) have unsaved changes! Close anyway?");
        doclose = QMessageBox::Yes == QMessageBox::warning( static_cast<QWidget*>(parent()), tr("Unsaved changes!"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if

    return doclose;
}   // end doBeforeAction


void ActionCloseAllFaceModels::doAction( Event)
{
    UndoStates::clear();
    for ( const FMV* fmv : MS::viewers())
    {
        const FMS fms = fmv->attached().models();   // Copy out since closing
        for ( const FM* fm : fms)
            FAM::close( fm);
    }   // end fmv
}   // end doAction


void ActionCloseAllFaceModels::doAfterAction( Event)
{
    MS::setInteractionMode(IMode::CAMERA_INTERACTION);
    emit onEvent( { Event::CLOSED_MODEL, Event::ALL_VIEWERS, Event::ALL_VIEWS, Event::VIEWER_CHANGE});
}   // end doAfterAction
