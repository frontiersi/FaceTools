/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionLoad.h>
#include <Action/ActionOrientCamera.h>
#include <Action/ActionRestoreLandmarks.h>
#include <FileIO/FaceModelManager.h>
#include <QFileInfo>
#include <QMessageBox>
using FaceTools::Action::ActionLoad;
using FaceTools::Action::Event;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionLoad::ActionLoad( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _loadHelper(nullptr), _dialog(nullptr)
{
#ifdef NDEBUG
    setAsync(true);
#endif
}   // end ctor


ActionLoad::~ActionLoad() { delete _loadHelper;}


void ActionLoad::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _loadHelper = new LoadFaceModelsHelper(p);
    _dialog = new QFileDialog(p, tr("Select model to load"));
    QStringList filters = FMM::fileFormats().createImportFilters();
    filters.prepend( "Any file (*)");
    _dialog->setNameFilters(filters);
    _dialog->setViewMode(QFileDialog::Detail);
    _dialog->setFileMode(QFileDialog::ExistingFile);    // Single file only
    //_dialog->setOption(QFileDialog::DontUseNativeDialog);
    //_dialog->setOption(QFileDialog::DontUseCustomDirectoryIcons);
}   // end postInit


bool ActionLoad::isAllowed( Event) { return FMM::numOpen() < FMM::loadLimit();}


// public
bool ActionLoad::load( const QString& fname)
{
    bool loaded = false;
    if ( isAllowed(Event::NONE))
    {
        QStringList fnames;
        fnames << fname;
        if ( _loadHelper->setFilteredFilenames(fnames) > 0)
            loaded = execute( Event::USER);
    }   // end if
    return loaded;
}   // end load


bool ActionLoad::doBeforeAction( Event)
{
    if ( _loadHelper->filenames().empty() && _dialog->exec())
        _loadHelper->setFilteredFilenames( _dialog->selectedFiles());

    const bool doLoad = _loadHelper->filenames().size() > 0;
    if ( doLoad)
    {
        QFileInfo finfo( _loadHelper->filenames().first());
        _dialog->setDirectory( finfo.absolutePath()); // Set directory for next load
        if ( _loadHelper->filenames().size() == 1)
            MS::showStatus( QString("Loading %1 ...").arg(finfo.absoluteFilePath()));
        else
            MS::showStatus( "Loading models ...");
    }   // end if
    return doLoad;
}   // end doBeforeAction


void ActionLoad::doAction( Event)
{
    _loadHelper->loadModels();
}   // end doAction


Event ActionLoad::doAfterAction( Event)
{
    _loadHelper->showLoadErrors();
    const FMS& fms = _loadHelper->lastLoaded();
    FM *fm = fms.empty() ? nullptr : *fms.begin();

    Event e = Event::NONE;
    if ( fm)
    {
        // Restore any missing landmarks and warn user of this.
        if ( fm->hasLandmarks() && ActionRestoreLandmarks::restoreMissingLandmarks( *fm))
        {
            static const QString msg = tr("Some more recently added landmarks were automatically set.<br>Ensure all landmark positions are correct before continuing.");
            QMB::information( static_cast<QWidget*>(parent()), tr("Some Landmarks Automatically Set"),
                              QString("<p align='center'>%1</p>").arg(msg));
        }   // end if

        // Get the viewer to place the newly loaded model into.
        FMV *fmv = MS::viewers()[1];
        if ( fmv->attached().size() > 0)
            fmv = MS::viewers()[2];
        if ( fmv->attached().size() > 0)
            fmv = MS::viewers()[0];
        if ( fmv->attached().size() > 1)
            fmv = MS::viewers()[1];
        if ( fmv->attached().size() > 1)
            fmv = MS::viewers()[2];
        if ( fmv->attached().size() > 1)
            fmv = MS::viewers()[0];
        if ( fmv->attached().size() > 2)
            fmv = MS::defaultViewer();

        FV *fv = MS::add( fm, fmv);
        MS::setSelected( fv);
        ActionOrientCamera::orient( fv, 1);

        MS::showStatus( "Finished loading.", 5000);
        e = Event::LOADED_MODEL | Event::MESH_CHANGE | Event::MODEL_SELECT;
    }   // end if
    else
    {
        MS::showStatus( "Error loading model!", 5000);
        e = Event::ERR;
    }   // end else

    return e;
}   // end doAfterAction
