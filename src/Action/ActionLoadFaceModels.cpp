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
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceControlSet;


ActionLoadFaceModels::ActionLoadFaceModels( const QString& dn, const QIcon& ico, const QKeySequence& ks, LoadFaceModelsHelper* lhelper)
    : FaceAction( dn, ico, ks), _loadHelper( lhelper)
{
    setAsync(true);
}   // end ctor


// public
bool ActionLoadFaceModels::loadModel( const QString& fname)
{
    bool loaded = false;
    if ( testEnabled())
    {
        QStringList fnames;
        fnames << fname;
        if ( _loadHelper->setFilteredFilenames(fnames) > 0)
            loaded = process();
    }   // end if
    return loaded;
}   // end loadModel


bool ActionLoadFaceModels::testEnabled() const { return !_loadHelper->reachedLoadLimit();}


bool ActionLoadFaceModels::doBeforeAction( FaceControlSet&)
{
    if ( _loadHelper->filenames().empty())
    {
        // Get the dialog filters
        QString anyf = "Any file (*.*)";
        QStringList filters = _loadHelper->createImportFilters().split(";;");
        filters.prepend(anyf);
        QString allf = filters.join(";;");

        QStringList fnames = QFileDialog::getOpenFileNames( _loadHelper->parentWidget(),
                                                            tr("Select one or more models to load"), "",
                                                            allf, &anyf);
        _loadHelper->setFilteredFilenames( fnames);
    }   // end if
    return !_loadHelper->filenames().empty();
}   // end doBeforeAction


bool ActionLoadFaceModels::doAction( FaceControlSet&)
{
    return _loadHelper->loadModels() > 0;   // Blocks
}   // end doAction


void ActionLoadFaceModels::doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool loaded)
{
    _loadHelper->showLoadErrors();
    if ( loaded)
        cs.insert(LOADED_MODEL);
}   // end doAfterAction
