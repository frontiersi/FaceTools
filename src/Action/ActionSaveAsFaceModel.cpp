/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <QFileDialog>
#include <QMessageBox>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSaveAsFaceModel;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;


ActionSaveAsFaceModel::ActionSaveAsFaceModel( FaceModelManager* fmm, QWidget *parent)
    : FaceAction(true/*this action disabled on other actions executing*/),
      _fmm(fmm), _parent(parent), _icon( ":/icons/SAVE_AS")
{
    addRespondTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( MODEL_TRANSFORMED);
    addRespondTo( MODEL_TEXTURE_CHANGED);
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);
    addRespondTo( FACE_NOTE_ADDED);
    addRespondTo( FACE_NOTE_DELETED);
    addRespondTo( FACE_NOTE_CHANGED);
    addRespondTo( MODEL_ORIENTATION_CHANGED);
    addRespondTo( MODEL_DESCRIPTION_CHANGED);
    addRespondTo( MODEL_SOURCE_CHANGED);
    setAsync(true);
}   // end ctor


bool ActionSaveAsFaceModel::doBeforeAction( FaceControlSet& fset)
{
    assert(fset.size() == 1);
    FaceModel* fm = fset.first()->data();
    // Make default save filename have the preferred extension
    std::string filename = _fmm->filepath( fm);
    boost::filesystem::path outpath( filename);
    const QString parentDir = outpath.parent_path().string().c_str();
    const QString dsuff = _fmm->fileFormats().getPreferredExt();
    filename = outpath.replace_extension( dsuff.toStdString()).string();

    QFileDialog fileDialog( _parent);
    fileDialog.setWindowTitle( tr("Save model as..."));
    fileDialog.setFileMode( QFileDialog::AnyFile);
    fileDialog.setNameFilters( _fmm->fileFormats().createExportFilters().split(";;"));
    fileDialog.setDirectory( parentDir);    // Default save directory is last save location for model
    fileDialog.setDefaultSuffix( dsuff);
    fileDialog.selectFile( filename.c_str());
    fileDialog.setAcceptMode( QFileDialog::AcceptSave);
    fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    QStringList fnames;
    if ( fileDialog.exec())
        fnames = fileDialog.selectedFiles();

    _filename = "";
    if ( !fnames.empty())
        _filename = fnames.first().toStdString();
    return !_filename.empty();
}   // end doBeforeAction


bool ActionSaveAsFaceModel::doAction( FaceControlSet& fset)
{
    assert(fset.size() == 1);
    assert( !_filename.empty());
    return !_fmm->write( fset.first()->data(), &_filename); // Save by specifying new filename
}   // end doAction


void ActionSaveAsFaceModel::doAfterAction( const FaceControlSet&, bool success)
{
    if (!success)
    {
        QString msg( ("\nUnable to save to \"" + _filename + "\"!").c_str());
        msg.append( ("\n" + _fmm->error()).c_str());
        QMessageBox::critical( _parent, tr("Unable to save file!"), tr(msg.toStdString().c_str()));
    }   // end if
    _filename = "";
}   // end doAfterAction


