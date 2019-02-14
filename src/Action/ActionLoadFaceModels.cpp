/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <FaceModelManager.h>
using FaceTools::Action::ActionLoadFaceModels;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::Vis::FV;
using FaceTools::FVS;


ActionLoadFaceModels::ActionLoadFaceModels( const QString& dn, const QIcon& ico, QWidget *p)
    : FaceAction( dn, ico), _loadHelper(p), _dialog( new QFileDialog(p, tr("Select one or more models to load")))
{
    setAsync(true);
    QStringList filters = _loadHelper.createImportFilters().split(";;");
    filters.prepend( "Any file (*)");
    _dialog->setNameFilters(filters);
    _dialog->setViewMode(QFileDialog::Detail);
    _dialog->setFileMode(QFileDialog::ExistingFiles);
    //_dialog->setOption(QFileDialog::DontUseCustomDirectoryIcons);
    _dialog->setOption(QFileDialog::DontUseNativeDialog);
}   // end ctor


bool ActionLoadFaceModels::testEnabled( const QPoint*) const
{
    using FaceTools::FileIO::FMM;
    return FMM::numOpen() < FMM::loadLimit();
}   // end testEnabled


// public
FV* ActionLoadFaceModels::loadModel( const QString& fname)
{
    bool loaded = false;
    if ( testEnabled(nullptr))
    {
        QStringList fnames;
        fnames << fname;
        if ( _loadHelper.setFilteredFilenames(fnames) > 0)
            loaded = process();
    }   // end if

    FV* fv = nullptr;
    if ( loaded)
        fv = _loadHelper.lastLoaded().first();
    return fv;
}   // end loadModel


bool ActionLoadFaceModels::doBeforeAction( FVS&, const QPoint&)
{
    if ( _loadHelper.filenames().empty())
    {
        if (_dialog->exec())
            _loadHelper.setFilteredFilenames( _dialog->selectedFiles());
    }   // end if
    return !_loadHelper.filenames().empty();
}   // end doBeforeAction


bool ActionLoadFaceModels::doAction( FVS& fvs, const QPoint&)
{
    fvs.clear();
    if ( _loadHelper.loadModels() > 0)   // Blocks
        fvs.insert(_loadHelper.lastLoaded());
    return !fvs.empty();
}   // end doAction


void ActionLoadFaceModels::doAfterAction( EventSet& cs, const FVS&, bool loaded)
{
    _loadHelper.showLoadErrors();
    if ( loaded)
        cs.insert(LOADED_MODEL);
}   // end doAfterAction
