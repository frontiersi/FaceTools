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

#include <ActionLoadFaceModels.h>
#include <QFileDialog>
using FaceTools::Action::ActionLoadFaceModels;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;


ActionLoadFaceModels::ActionLoadFaceModels( LoadFaceModelsHelper* lhelper)
    : FaceAction( true), _loadHelper( lhelper),
      _icon( ":/icons/LOAD"), _scut( Qt::CTRL + Qt::Key_O)
{
    setAsync(true);
}   // end ctor


bool ActionLoadFaceModels::testEnabled() { return !_loadHelper->reachedLoadLimit();}


bool ActionLoadFaceModels::doBeforeAction( FaceControlSet&)
{
    // Get the dialog filters
    QString anyf = "Any file (*.*)";
    QStringList filters = _loadHelper->createImportFilters().split(";;");
    filters.prepend(anyf);
    QString allf = filters.join(";;");

    // Don't use native dialog because there's some Windows 10 debug output stating that some element of the
    // dialog couldn't be found. On some Win10 machines, crashes occur unless non-native dialogs are used.
    QStringList fnames = QFileDialog::getOpenFileNames( _loadHelper->parentWidget(),
                                                        tr("Select one or more models to load"), "",
                                                        allf, &anyf, QFileDialog::DontUseNativeDialog);
    return _loadHelper->setFilteredFilenames( fnames) > 0;
}   // end doBeforeAction


bool ActionLoadFaceModels::doAction( FaceControlSet&/*ignored*/) { return _loadHelper->loadModels() > 0;}
void ActionLoadFaceModels::doAfterAction( const FaceControlSet&, bool success) { _loadHelper->showLoadErrors();}
