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

#include <ActionLoadFaceModels.h>
#include <FaceModelManager.h>
using FaceTools::Action::ActionLoadFaceModels;
using FaceTools::Action::Event;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FileIO::FMM;
using MS = FaceTools::Action::ModelSelector;


ActionLoadFaceModels::ActionLoadFaceModels( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _loadHelper(nullptr), _dialog(nullptr)
{
    setAsync(true);
}   // end ctor


ActionLoadFaceModels::~ActionLoadFaceModels()
{
    delete _loadHelper;
}   // end dtor


void ActionLoadFaceModels::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _loadHelper = new LoadFaceModelsHelper(p);
    _dialog = new QFileDialog(p, tr("Select one or more models to load"));
    QStringList filters = FMM::fileFormats().createImportFilters().split(";;");
    filters.prepend( "Any file (*)");
    _dialog->setNameFilters(filters);
    _dialog->setViewMode(QFileDialog::Detail);
    _dialog->setFileMode(QFileDialog::ExistingFiles);
    _dialog->setOption(QFileDialog::DontUseNativeDialog);
    _dialog->setOption(QFileDialog::DontUseCustomDirectoryIcons);
}   // end postInit


bool ActionLoadFaceModels::checkEnable( Event)
{
    return FMM::numOpen() < FMM::loadLimit();
}   // end checkEnable


// public
bool ActionLoadFaceModels::loadModel( const QString& fname)
{
    bool loaded = false;
    if ( checkEnable(Event::NONE))
    {
        QStringList fnames;
        fnames << fname;
        if ( _loadHelper->setFilteredFilenames(fnames) > 0)
            loaded = execute( Event::USER);
    }   // end if
    return loaded;
}   // end loadModel


bool ActionLoadFaceModels::doBeforeAction( Event)
{
    if ( _loadHelper->filenames().empty())
    {
        if (_dialog->exec())
            _loadHelper->setFilteredFilenames( _dialog->selectedFiles());
    }   // end if

    const int nfiles = _loadHelper->filenames().size();
    const bool doLoad = nfiles > 0;
    if ( doLoad)
        MS::showStatus( QString("Loading model%1...").arg(nfiles > 1 ? "s" : ""));
    return doLoad;
}   // end doBeforeAction


void ActionLoadFaceModels::doAction( Event)
{
    _loadHelper->loadModels();
}   // end doAction


void ActionLoadFaceModels::doAfterAction( Event)
{
    _loadHelper->showLoadErrors();
    const FMS& fms = _loadHelper->lastLoaded();
    for ( FM* fm : fms)
        MS::addFaceView( fm);
    if ( !fms.empty())
    {
        MS::showStatus( QString("Finished loading model%1.").arg( fms.size() > 1 ? "s" : ""), 5000);
        emit onEvent( {Event::LOADED_MODEL, Event::GEOMETRY_CHANGE});
    }   // end if
    else
        MS::showStatus( "Error loading model(s)!", 5000);
}   // end doAfterAction
