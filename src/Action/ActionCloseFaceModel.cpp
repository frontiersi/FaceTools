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

#include <Action/ActionCloseFaceModel.h>
#include <Action/FaceActionManager.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
#include <boost/filesystem.hpp>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCloseFaceModel;
using FaceTools::Action::Event;
using FaceTools::Action::FAM;
using FaceTools::FM;
using FaceTools::FileIO::FMM;
using MS = FaceTools::Action::ModelSelector;


ActionCloseFaceModel::ActionCloseFaceModel( const QString& dname, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dname, ico, ks) {}

bool ActionCloseFaceModel::isAllowed( Event) { return MS::isViewSelected();}


bool ActionCloseFaceModel::doBeforeAction( Event)
{
    const FM* fm = MS::selectedModel();
    assert(fm);
    fm->lockForRead();
    bool inPreferredFormat = FMM::hasPreferredFileFormat(fm);

    // If FaceModel hasn't been saved and the user doesn't want to close it (after prompting), remove from action set.
    bool doclose = false;
    if ( fm->isSaved())
        doclose = true;
    else
    {
        const std::string fname = boost::filesystem::path( FMM::filepath(fm)).filename().string();
        QString msg = tr( ("Model '" + fname + "' is unsaved! Really close?").c_str());
        if ( fm->hasMetaData() && !inPreferredFormat)
            msg = tr("Not saved as 3DF; data will be lost! Really close?");
        doclose = QMessageBox::Yes == QMessageBox::warning( static_cast<QWidget*>(parent()), tr("Unsaved Data!"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if
    fm->unlock();

    return doclose;
}   // end doBeforeAction


void ActionCloseFaceModel::doAction( Event)
{
    const FM *fm = MS::selectedModel();
    UndoStates::clear( fm);
    FAM::close( fm);
}   // end doAction


Event ActionCloseFaceModel::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return Event::CLOSED_MODEL | Event::VIEWER_CHANGE | Event::ALL_VIEWERS;
}   // end doAfterAction
