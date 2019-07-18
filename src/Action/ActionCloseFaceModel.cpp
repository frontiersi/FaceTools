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

#include <ActionCloseFaceModel.h>
#include <FaceActionManager.h>
#include <FaceModelManager.h>
#include <QMessageBox>
#include <boost/filesystem.hpp>
#include <algorithm>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCloseFaceModel;
using FaceTools::Action::Event;
using FaceTools::Action::FAM;
using FaceTools::FM;
using FaceTools::FileIO::FMM;
using MS = FaceTools::Action::ModelSelector;


ActionCloseFaceModel::ActionCloseFaceModel( const QString& dname, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dname, ico, ks) {}

bool ActionCloseFaceModel::checkEnable( Event) { return MS::selectedView();}


bool ActionCloseFaceModel::doBeforeAction( Event)
{
    const FM* fm = MS::selectedModel();
    fm->lockForRead();
    bool inPreferredFormat = FMM::hasPreferredFileFormat(fm);

    // If FaceModel hasn't been saved and the user doesn't want to close it (after prompting), remove from action set.
    bool doclose = false;
    if ( fm->isSaved())
        doclose = true;
    else
    {
        const std::string fname = boost::filesystem::path( FMM::filepath(fm)).filename().string();
        QString msg = tr( ("Model '" + fname + "' has unsaved changes! Close anyway?").c_str());
        if ( fm->hasMetaData() && !inPreferredFormat)
            msg = tr("Not saved in .3df file format; landmark and other meta-data will be lost! Close anyway?");

        doclose = QMessageBox::Yes == QMessageBox::warning( static_cast<QWidget*>(parent()), tr("Unsaved changes!"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if
    fm->unlock();

    return doclose;
}   // end doBeforeAction


void ActionCloseFaceModel::doAction( Event)
{
    UndoStates::clear( MS::selectedModel());
    FAM::close( MS::selectedModel());
}   // end doAction


void ActionCloseFaceModel::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    emit onEvent( {Event::CLOSED_MODEL, Event::VIEWER_CHANGE});
}   // end doAfterAction
