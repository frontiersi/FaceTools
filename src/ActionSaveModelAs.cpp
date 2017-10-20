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

#include <ActionSaveModelAs.h>
#include <FaceModelManager.h>   // FaceTools
#include <QFileDialog>
#include <QMessageBox>
#include <boost/filesystem.hpp>
using FaceTools::ActionSaveModelAs;
using FaceTools::ModelInteractor;
using FaceTools::FaceModel;
typedef FaceTools::FaceModelManager FMM;

// public
ActionSaveModelAs::ActionSaveModelAs( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _dname( "Save &As..."),
      _keyseq( Qt::SHIFT + Qt::CTRL + Qt::Key_S),
      _fmodel(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionSaveModelAs::setInteractive( ModelInteractor* interactor, bool enable)
{
    _fmodel = NULL;
    if ( !enable)
        interactor->getModel()->disconnect( this);
    else
    {
        _fmodel = interactor->getModel();
        connect( _fmodel, &FaceModel::onClearedUndos, this, &ActionSaveModelAs::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractive


void ActionSaveModelAs::checkEnable()
{
    setEnabled( _fmodel && _fmodel->hasUndos());
}   // end checkEnable


// protected
bool ActionSaveModelAs::doAction()
{
    assert( _fmodel);
    std::string outfile = _fmodel->getSaveFilepath();
    boost::filesystem::path outpath(outfile);
    const QString parentDir = outpath.parent_path().string().c_str();

    // Make outfile have the preferred extension
    const QString defaultSuffix = FMM::get().getPrimaryExt().c_str();
    outfile = outpath.replace_extension( defaultSuffix.toStdString()).string();

    QFileDialog fileDialog;
    fileDialog.setWindowTitle( tr("Save Model"));
    fileDialog.setFileMode( QFileDialog::AnyFile);
    fileDialog.setNameFilters( FMM::get().getExportFilters());
    fileDialog.setDirectory( parentDir);    // Default save directory is last save location for model
    fileDialog.setDefaultSuffix( defaultSuffix);
    fileDialog.selectFile( outfile.c_str());
    fileDialog.setAcceptMode( QFileDialog::AcceptSave);
    fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    QStringList fileNames;
    if ( fileDialog.exec())
        fileNames = fileDialog.selectedFiles();
    if ( !fileNames.empty())
        outfile = fileNames.first().toStdString();

    if ( outfile.empty())
        return false;

    return FMM::get().save( _fmodel, outfile);
}   // end doAction


