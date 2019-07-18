/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionSaveFaceModel.h>
#include <FaceModelManager.h>
#include <QMessageBox>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSaveFaceModel;
using FaceTools::Action::Event;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::FMM;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionSaveFaceModel::ActionSaveFaceModel( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    setAsync(true);
}   // end ctor


bool ActionSaveFaceModel::checkEnable( Event)
{
    if ( !MS::isViewSelected())
        return false;
    const FM *fm = MS::selectedModel();
    fm->lockForRead();
    const bool isrdy = !fm->isSaved() && (FMM::hasPreferredFileFormat(fm) || !fm->hasMetaData());
    fm->unlock();
    return isrdy;
}   // end testReady


bool ActionSaveFaceModel::doBeforeAction( Event)
{
    MS::showStatus("Saving...");
    return true;
}   // end doBeforeAction


void ActionSaveFaceModel::doAction( Event)
{
    _egrp.clear();
    assert(_fails.empty());
    FM *fm = MS::selectedModel();
    fm->lockForWrite();
    if ( fm->landmarks().empty())
    {
        fm->fixOrientation();
        _egrp.add(Event::ORIENTATION_CHANGE);
    }   // end if
    fm->unlock();
    fm->lockForRead();
    std::string filepath;   // Will be the last saved filepath
    const bool wokay = FMM::write( fm, &filepath);  // Save using current filepath for the model
    fm->unlock();
    if ( wokay)
    {
        _egrp.add(Event::SAVED_MODEL);
        UndoStates::clear(fm);
    }   // end if
    else
        _fails[FMM::error()] << filepath.c_str();
}   // end doAction


void ActionSaveFaceModel::doAfterAction( Event)
{
    if ( _fails.empty())
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        const QString fpath = FMM::filepath( MS::selectedModel()).c_str();
        MS::showStatus( QString("Saved to '%1'").arg(fpath), 5000);
        emit onEvent( _egrp);
    }   // end if
    else
    {
        for ( auto f : _fails)  // Display a critical error for each type of error message received
        {
            MS::showStatus( "Failed to save model!", 10000);
            QString msg( (f.first + "\nUnable to save the following:\n").c_str());
            msg.append( f.second.join("\n"));
            QMessageBox::critical( static_cast<QWidget*>(parent()), tr("Unable to save file(s)!"), tr(msg.toStdString().c_str()));
        }   // end for
        _fails.clear(); // Ensure the fail set is cleared
    }   // end else
}   // end doAfterAction
