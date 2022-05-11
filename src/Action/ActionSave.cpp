/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionSave.h>
#include <FileIO/FaceModelDatabase.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionSave;
using FaceTools::Action::Event;
using FaceTools::Action::FaceAction;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionSave::ActionSave( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _saveAs( nullptr)
{
    // Note need to refresh after SAVE since SaveAs will cause this event.
    addRefreshEvent( Event::MODEL_SELECT | Event::MESH_CHANGE | Event::AFFINE_CHANGE | Event::SAVED_MODEL
                   | Event::LANDMARKS_CHANGE | Event::PATHS_CHANGE | Event::METADATA_CHANGE);
    //setAsync(true);   // Don't allow this to be asynchronous (cause of database thread for FMM::write).
}   // end ctor


bool ActionSave::isAllowed( Event)
{
    if ( !MS::isViewSelected())
        return false;
    FM::RPtr fm = MS::selectedModelScopedRead();
    bool isrdy = !fm->isSaved();
    if ( !_saveAs)
        isrdy = !fm->isSaved() && ( FMM::hasPreferredFileFormat(*fm) || !fm->hasMetaData());
    return isrdy;
}   // end testReady


bool ActionSave::doBeforeAction( Event e)
{
    bool isNormalSave = true;
    FM::RPtr fm = MS::selectedModelScopedRead();
    if ( _saveAs && !FMM::hasPreferredFileFormat(*fm) && fm->hasMetaData())
    {
        isNormalSave = false;   // Will cause this action to cancel and allow the SaveAs action to complete.
        _saveAs->execute(e);
    }   // end if
    else
        MS::showStatus( "Saving...", 0, true);
    return isNormalSave;
}   // end doBeforeAction


void ActionSave::doAction( Event)
{
    _egrp = Event::NONE;
    assert(_fails.empty());
    FM::WPtr fm = MS::selectedModelScopedWrite();
    if ( !fm->hasMask())
    {
        _egrp |= Event::MESH_CHANGE | Event::AFFINE_CHANGE;
        fm->fixTransformMatrix();
    }   // end if
    QString filepath;   // Will be the last saved filepath
    const bool wokay = FMM::write( *fm, filepath);  // Save using current filepath for the model
    if ( wokay)
        _egrp |= Event::SAVED_MODEL;
    else
    {
        _fails[FMM::error()] << filepath;
        _egrp |= Event::ERR;
    }   // end else
}   // end doAction


Event ActionSave::doAfterAction( Event)
{
    if ( _fails.empty())
    {
        FM *fm = MS::selectedModel();
        UndoStates::clear(fm);
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        const QString fpath = FMM::filepath( *fm);
        MS::showStatus( QString("Saved to '%1'").arg(fpath), 5000);
    }   // end if
    else
    {
        for ( auto f : _fails)  // Display a critical error for each type of error message received
        {
            MS::showStatus( "Failed to save model!", 10000);
            QString msg = tr( (f.first.toStdString() + "<br>Unable to save the following:<br>").c_str());
            msg.append( f.second.join("<br>"));
            QMB::critical( static_cast<QWidget*>(parent()), tr("File Save Error!"),
                            QString("<p align='center'>%1</p>").arg(msg));
        }   // end for
        _fails.clear(); // Ensure the fail set is cleared
    }   // end else
    return _egrp;
}   // end doAfterAction
