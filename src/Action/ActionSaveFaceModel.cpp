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

#include <Action/ActionSaveFaceModel.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSaveFaceModel;
using FaceTools::Action::Event;
using FaceTools::Action::FaceAction;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::Action::ModelSelector;


ActionSaveFaceModel::ActionSaveFaceModel( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _saveAs( nullptr)
{
}   // end ctor


bool ActionSaveFaceModel::isAllowed( Event)
{
    if ( !MS::isViewSelected())
        return false;
    const FM *fm = MS::selectedModel();
    fm->lockForRead();
    bool isrdy = !fm->isSaved();
    if ( !_saveAs)
        isrdy = !fm->isSaved() && ( FMM::hasPreferredFileFormat(fm) || !fm->hasMetaData());
    fm->unlock();
    return isrdy;
}   // end testReady


bool ActionSaveFaceModel::doBeforeAction( Event e)
{
    bool isNormalSave = true;
    const FM *fm = MS::selectedModel();
    if ( _saveAs && !FMM::hasPreferredFileFormat(fm) && fm->hasMetaData())
    {
        isNormalSave = false;   // Will cause this action to cancel and allow the SaveAs action to complete.
        _saveAs->execute(e);
    }   // end if
    else
        MS::showStatus( "Saving...", 0, true);
    return isNormalSave;
}   // end doBeforeAction


void ActionSaveFaceModel::doAction( Event)
{
    _egrp = Event::NONE;
    assert(_fails.empty());
    FM *fm = MS::selectedModel();
    fm->lockForWrite();
    if ( !fm->hasMask())
    {
        _egrp |= Event::MESH_CHANGE;
        fm->fixTransformMatrix();
    }   // end if
    QString filepath;   // Will be the last saved filepath
    const bool wokay = FMM::write( fm, filepath);  // Save using current filepath for the model
    fm->unlock();
    if ( wokay)
    {
        _egrp |= Event::SAVED_MODEL;
        UndoStates::clear(fm);
    }   // end if
    else
    {
        _fails[FMM::error()] << filepath;
        _egrp |= Event::ERR;
    }   // end else
}   // end doAction


Event ActionSaveFaceModel::doAfterAction( Event)
{
    if ( _fails.empty())
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        const QString fpath = FMM::filepath( MS::selectedModel());
        MS::showStatus( QString("Saved to '%1'").arg(fpath), 5000);
    }   // end if
    else
    {
        for ( auto f : _fails)  // Display a critical error for each type of error message received
        {
            MS::showStatus( "Failed to save model!", 10000);
            QString msg = tr( (f.first.toStdString() + "\nUnable to save the following:\n").c_str());
            msg.append( f.second.join("\n"));
            QMessageBox::critical( static_cast<QWidget*>(parent()), tr("Unable to save file(s)!"), msg);
        }   // end for
        _fails.clear(); // Ensure the fail set is cleared
    }   // end else
    return _egrp;
}   // end doAfterAction
