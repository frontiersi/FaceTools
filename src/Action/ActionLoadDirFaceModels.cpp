/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionLoadDirFaceModels.h>
#include <FileIO/FaceModelManager.h>
using FaceTools::Action::ActionLoadDirFaceModels;
using FaceTools::Action::Event;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FileIO::FMM;
using MS = FaceTools::Action::ModelSelector;


ActionLoadDirFaceModels::ActionLoadDirFaceModels( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _loadHelper(nullptr), _dialog(nullptr)
{
    setAsync(true);
}   // end ctor


ActionLoadDirFaceModels::~ActionLoadDirFaceModels()
{
    delete _loadHelper;
}   // end dtor


void ActionLoadDirFaceModels::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _loadHelper = new LoadFaceModelsHelper(p);
    _dialog = new QFileDialog(p, tr("Select directory containing models"));
    _dialog->setViewMode(QFileDialog::Detail);
    _dialog->setFileMode(QFileDialog::Directory);
    _dialog->setOption(QFileDialog::ShowDirsOnly);
    //_dialog->setOption(QFileDialog::DontUseNativeDialog);
    //_dialog->setOption(QFileDialog::DontUseCustomDirectoryIcons);
}   // end postInit


bool ActionLoadDirFaceModels::isAllowed( Event)
{
    return FMM::numOpen() < FMM::loadLimit();
}   // end isAllowedd


bool ActionLoadDirFaceModels::doBeforeAction( Event)
{
    if ( !_dialog->exec())
        return false;
    QDir qdir = _dialog->directory();
    QStringList fnames = qdir.entryList( FMM::fileFormats().createSimpleImportFilters());
    std::for_each( std::begin(fnames), std::end(fnames), [&](QString& fn){ fn = QDir::cleanPath( qdir.filePath(fn));});
    const int nfiles = fnames.size();
    const bool doLoad = _loadHelper->setFilteredFilenames( fnames) > 0;
    if ( doLoad)
        MS::showStatus( QString("Loading model%1...").arg(nfiles > 1 ? "s" : ""));
    return doLoad;
}   // end doBeforeAction


void ActionLoadDirFaceModels::doAction( Event)
{
    _loadHelper->loadModels();
}   // end doAction


Event ActionLoadDirFaceModels::doAfterAction( Event)
{
    _loadHelper->showLoadErrors();
    const FMS& fms = _loadHelper->lastLoaded();
    FV *fv = nullptr;
    for ( FM* fm : fms)
        fv = MS::add( fm, MS::defaultViewer());
    if ( fv)
        MS::setSelected(fv);

    Event e;
    if ( !fms.empty())
    {
        MS::showStatus( QString("Finished loading model%1.").arg( fms.size() > 1 ? "s" : ""), 5000);
        e = Event::LOADED_MODEL | Event::MESH_CHANGE;
    }   // end if
    else
    {
        MS::showStatus( "Error loading model(s)!", 5000);
        e = Event::ERR;
    }   // end else

    return e;
}   // end doAfterAction
