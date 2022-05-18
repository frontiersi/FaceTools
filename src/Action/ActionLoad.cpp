/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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
#include <FileIO/FaceModelDatabase.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
#include <QFileInfo>
using FaceTools::Action::ActionLoad;
using FaceTools::Action::Event;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::Vis::FV;
using FMD = FaceTools::FileIO::FaceModelDatabase;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionLoad::ActionLoad( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _loadHelper(nullptr), _dialog(nullptr)
{
    setAsync(true);
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


bool ActionLoad::isAllowed( Event) { return !FMM::loadLimitReached();}


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
    // Set filename to load from the dialog if the file isn't already set
    if ( _loadHelper->filenames().empty() && _dialog->exec())
        _loadHelper->setFilteredFilenames( _dialog->selectedFiles());

    const bool doLoad = _loadHelper->filenames().size() > 0;
    if ( doLoad)
    {
        const QFileInfo finfo( _loadHelper->filenames().first());
        _dialog->setDirectory( finfo.absolutePath()); // Set directory for next load
        const QString loadText = QString("Loading %1 ...").arg(finfo.absoluteFilePath());
        MS::showStatus( loadText);
        std::cerr << loadText.toStdString() << std::endl;
    }   // end if
    return doLoad;
}   // end doBeforeAction


void ActionLoad::doAction( Event) { _loadHelper->loadModels();}


Event ActionLoad::doAfterAction( Event)
{
    _loadHelper->showLoadErrors();
    const FMS& fms = _loadHelper->lastLoaded();
    FM *fm = fms.empty() ? nullptr : *fms.begin();

    Event e = Event::NONE;
    if ( fm)
    {
        QString msg;
        // Restore any missing landmarks and warn user of this.
        if ( fm->hasLandmarks() && ActionRestoreLandmarks::restoreMissingLandmarks( *fm))
        {
            msg = tr("Some new landmarks were added.<br>Confirm their positions and save before continuing.");
            QMB::information( static_cast<QWidget*>(parent()), tr("New Landmarks Added"),
                              QString("<p align='center'>%1</p>").arg(msg));
        }   // end if

        const QString fpath = FMM::filepath( *fm);
        if ( FMM::isPreferredFileFormat( fpath))
            FMD::refreshImage( *fm, fpath); // setMetaSaved may be false after this

        // If subject metadata are not saved (and it wasn't because of new landmarks),
        // it's because the database updated info from a different model.
        if ( msg.isEmpty() && !fm->isMetaSaved())
        {
            msg = tr("This subject's details were updated from the database.<br>Confirm and save before continuing.");
            QMB::information( static_cast<QWidget*>(parent()), tr("Subject Details Updated"),
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
