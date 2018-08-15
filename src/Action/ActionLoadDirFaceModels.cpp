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

#include <ActionLoadDirFaceModels.h>
#include <QFileDialog>
using FaceTools::Action::ActionLoadDirFaceModels;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceControlSet;


ActionLoadDirFaceModels::ActionLoadDirFaceModels( const QString& dn, const QIcon& ico, LoadFaceModelsHelper* lhelper)
    : FaceAction( dn, ico), _loadHelper( lhelper)
{
    setAsync(true);
}   // end ctor


bool ActionLoadDirFaceModels::doBeforeAction( FaceControlSet&, const QPoint&)
{
    // Don't use native dialog because there's some Windows 10 debug output stating that some element of the
    // dialog couldn't be found. On some Win10 machines, crashes occur unless non-native dialogs are used.
    QString dname = QFileDialog::getExistingDirectory( _loadHelper->parentWidget(),
                                                       tr("Select directory containing models"), "",
                                                       QFileDialog::ShowDirsOnly);
                                                       //QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
    // Get list of filenames from directory
    QDir qdir(dname);
    QStringList fnames = qdir.entryList( _loadHelper->createSimpleImportFilters());
    std::for_each( std::begin(fnames), std::end(fnames), [&](auto& fn){ fn = QDir::cleanPath( qdir.filePath(fn));});
    size_t nfiles = _loadHelper->setFilteredFilenames( fnames);
    return nfiles > 0;
}   // end doBeforeAction


bool ActionLoadDirFaceModels::doAction( FaceControlSet&, const QPoint&)
{
    return _loadHelper->loadModels() > 0;
}   // end doAction


void ActionLoadDirFaceModels::doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool loaded)
{
    _loadHelper->showLoadErrors();
    if ( loaded)
        cs.insert(LOADED_MODEL);
}   // end doAfterAction
