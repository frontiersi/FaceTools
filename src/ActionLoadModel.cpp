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

#include <ActionLoadModel.h>
#include <FaceModelManager.h>   // FaceTools
#include <QFileDialog>
#include <QMessageBox>
using FaceTools::ActionLoadModel;
typedef FaceTools::FaceModelManager FMM;


// public
ActionLoadModel::ActionLoadModel( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _keyseq( Qt::CTRL + Qt::Key_O)
{
    init();
    setEnabled(true);
}   // end ctor


// protected
bool ActionLoadModel::doAction()
{
    QFileDialog fileDialog;
    fileDialog.setWindowTitle( tr("Open face model file"));
    fileDialog.setFileMode( QFileDialog::ExistingFile);
    fileDialog.setNameFilters( FMM::get().getImportFilters());
    fileDialog.setDirectory( "");
    fileDialog.setAcceptMode( QFileDialog::AcceptOpen);
    // Don't use native dialog because there's some Windows 10 debug output stating that
    // some element of the dialog couldn't be found. On some Win10 machines, crashes occur
    // unless non-native dialogs are used.
    // As at 01/02/17, this behaviour needs rechecking on different Win10 machines.
    //fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    QStringList fileNames;
    if ( fileDialog.exec())
        fileNames = fileDialog.selectedFiles();
    QString infile;
    if ( !fileNames.empty())
        infile = fileNames.at(0);

    if ( infile.isEmpty())
        return false;

    return FMM::get().load( infile.toStdString());
}   // end doAction

