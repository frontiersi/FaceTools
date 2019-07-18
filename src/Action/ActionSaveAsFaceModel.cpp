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

#include <ActionSaveAsFaceModel.h>
#include <FaceModelManager.h>
#include <QFileDialog>
#include <QMessageBox>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSaveAsFaceModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FileIO::FMM;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionSaveAsFaceModel::ActionSaveAsFaceModel( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    setAsync(true);
}   // end ctor


QString ActionSaveAsFaceModel::whatsThis() const
{
    QStringList htext;
    htext << "Save the selected model to a specific file format. Note that metadata";
    htext << "such as landmarks and demographic data can only be saved in the default";
    htext << QString("file format (%1).").arg(FMM::fileFormats().preferredExt());
    return htext.join(" ");
}   // end whatsThis


bool ActionSaveAsFaceModel::checkEnable( Event) { return MS::isViewSelected();}


bool ActionSaveAsFaceModel::doBeforeAction( Event)
{
    const FM* fm = MS::selectedModel();

    // Make default save filename have the preferred extension
    std::string filename = FMM::filepath( fm);
    boost::filesystem::path outpath( filename);
    const QString parentDir = outpath.parent_path().string().c_str();
    const QString dsuff = FMM::fileFormats().preferredExt();
    filename = outpath.replace_extension( dsuff.toStdString()).string();

    QFileDialog fileDialog( static_cast<QWidget*>(parent()));
    fileDialog.setWindowTitle( tr("Save model as..."));
    fileDialog.setFileMode( QFileDialog::AnyFile);
    const QStringList filters = FMM::fileFormats().createExportFilters().split(";;");
    fileDialog.setNameFilters( filters);
    fileDialog.setDirectory( parentDir);    // Default save directory is last save location for model
    fileDialog.setDefaultSuffix( dsuff);
    fileDialog.selectFile( filename.c_str());
    fileDialog.setAcceptMode( QFileDialog::AcceptSave);
    fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    QWidget* prnt = static_cast<QWidget*>(parent());
    while ( _filename.empty())
    {
        QStringList fnames;
        if ( fileDialog.exec())
            fnames = fileDialog.selectedFiles();

        _filename = "";
        if ( !fnames.empty())
            _filename = fnames.first().toStdString();
        else
            break;

        if ( !FMM::hasFileHandler( _filename))
        {
            static const QString msg = tr("Not a valid file format; please choose one of the listed formats.");
            QMessageBox::information( prnt, tr("Invalid File Format"), msg);
            _filename = "";
            continue;
        }   // end if

        // Display a warning if the model has meta-data and the selected format is not XML based.
        if ( fm->hasMetaData() && !FMM::isPreferredFileFormat( _filename))
        {
            static const QString msg0 = tr("Landmarks and other metadata are not saved in this file format; continue?");
            static const QString msg1 = tr("Model texture, landmarks and other metadata are not saved in this file format; continue?");
            const QString* msg = &msg0;
            if ( !FMM::canSaveTextures(_filename))
                msg = &msg1;
            if ( QMessageBox::No == QMessageBox::question( prnt, tr("Ignore Metadata?"), *msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                _filename = "";
        }   // end if
    }   // end while

    if ( !_filename.empty())
        MS::showStatus( QString( "Saving model to '%1'...").arg(_filename.c_str()));

    return !_filename.empty();
}   // end doBeforeAction


void ActionSaveAsFaceModel::doAction( Event)
{
    _egrp.clear();
    assert( !_filename.empty());
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    if ( fm->landmarks().empty())   // Save the model's position and orientation if landmarks aren't set.
    {
        fm->fixOrientation();
        _egrp.add(Event::ORIENTATION_CHANGE);
    }   // end if
    fm->unlock();
    fm->lockForRead();
    const bool wokay = FMM::write( fm, &_filename); // Save by specifying new filename
    fm->unlock();
    if ( wokay)
    {
        _egrp.add(Event::SAVED_MODEL);
        UndoStates::clear( fm);
    }   // end if
}   // end doAction


void ActionSaveAsFaceModel::doAfterAction( Event)
{
    if ( _egrp.has(Event::SAVED_MODEL))
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        MS::showStatus( QString("Saved to '%1'").arg(_filename.c_str()), 5000);
        emit onEvent( _egrp);
    }   // end if
    else
    {
        QString msg( ("\nUnable to save to '" + _filename + "'!").c_str());
        MS::showStatus( msg, 10000);
        msg.append( ("\n" + FMM::error()).c_str());
        QMessageBox::critical( static_cast<QWidget*>(parent()), tr("Unable to save file!"), tr(msg.toStdString().c_str()));
    }   // end else
    _filename = "";
}   // end doAfterAction

