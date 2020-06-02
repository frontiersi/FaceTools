/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionSaveAsFaceModel.h>
#include <FileIO/FaceModelManager.h>
#include <QMessageBox>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSaveAsFaceModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;
using QMB = QMessageBox;
namespace BFS = boost::filesystem;


ActionSaveAsFaceModel::ActionSaveAsFaceModel( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _fdialog(nullptr)
{
}   // end ctor


void ActionSaveAsFaceModel::postInit()
{
    _fdialog = new QFileDialog( static_cast<QWidget*>(parent()));
    _fdialog->setWindowTitle( tr("Save model as..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setViewMode( QFileDialog::Detail);
    _fdialog->setAcceptMode( QFileDialog::AcceptSave);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
}   // end postInit


QString ActionSaveAsFaceModel::whatsThis() const
{
    QStringList htext;
    htext << "Save the selected model to a specific file format. Note that data";
    htext << "such as landmarks and measurements can only be saved in";
    htext << QString("%1 format.").arg(FileIO::FMM::fileFormats().preferredExt());
    return htext.join(" ");
}   // end whatsThis


bool ActionSaveAsFaceModel::isAllowed( Event) { return MS::isViewSelected();}


bool ActionSaveAsFaceModel::doBeforeAction( Event)
{
    _fdialog->setNameFilters( FileIO::FMM::fileFormats().createExportFilters());

    const FM* fm = MS::selectedModel();
    BFS::path outpath( FileIO::FMM::filepath(fm));

    // Default save directory is last save location for model
    const QString parentPath = QString::fromStdString( outpath.parent_path().string());
    _fdialog->setDirectory( parentPath);

    // Make default save filename have the preferred extension
    const std::string filename = outpath.replace_extension( FileIO::FMM::fileFormats().preferredExt().toStdString()).string();
    _fdialog->selectFile( QString::fromStdString(filename));

    QWidget* prnt = static_cast<QWidget*>(parent());
    while ( _filename.empty())
    {
        QStringList fnames;
        if ( _fdialog->exec())
            fnames = _fdialog->selectedFiles();

        _discardTexture = false;
        _filename = fnames.empty() ? "" : fnames.first().trimmed().toStdString();
        if ( _filename.empty())
            break;

        // If _filename does not have an extension, use the currently selected one.
        BFS::path fpath(_filename);
        if ( !fpath.has_extension())
        {
            const QString cext = FileIO::FMM::fileFormats().extensionForFilter( _fdialog->selectedNameFilter());
            _filename = fpath.replace_extension( cext.toStdString()).string();
        }   // end if

        if ( !FileIO::FMM::canWrite( _filename))
        {
            std::cerr << "Can't write to " << _filename << "; invalid file format" << std::endl;
            static const QString msg = tr("Not a valid file format; please choose one of the listed formats.");
            QMB::information( prnt, tr("Invalid File Format"), msg);
            _filename = "";
            continue;
        }   // end if

        // Warn if model has meta-data or texture and the selected file type cannot save those things.
        const bool mayLoseMetaData = fm->hasMetaData() && !FileIO::FMM::isPreferredFileFormat( _filename);
        const bool mayLoseTexture = fm->hasTexture() && !FileIO::FMM::canSaveTextures( _filename);
        if ( mayLoseMetaData || mayLoseTexture)
        {
            static const QString tit0 = tr("Discard Metadata?!");
            static const QString msg0 = tr("Metadata will not be saved in this format! Continue?");
            static const QString tit1 = tr("Discard Texture?!");
            static const QString msg1 = tr("Texture will not be saved in this format! Continue?");
            static const QString tit2 = tr("Discard Texture and Metadata?!");
            static const QString msg2 = tr("Texture and metadata will not be saved in this format! Continue?");

            const QString* tit = &tit0;
            const QString* msg = &msg0;
            if ( mayLoseTexture)
            {
                tit = &tit1;
                msg = &msg1;
                _discardTexture = true;
                if ( fm->hasMetaData())
                {
                    tit = &tit2;
                    msg = &msg2;
                }   // end if
            }   // end if

            if ( QMB::No == QMB::warning( prnt, *tit, *msg, QMB::Yes | QMB::No, QMB::Yes))
                _filename = "";
        }   // end if
    }   // end while

    if ( !_filename.empty())
        MS::showStatus( QString( "Saving model to '%1'...").arg(_filename.c_str()));

    return !_filename.empty();
}   // end doBeforeAction


void ActionSaveAsFaceModel::doAction( Event)
{
    _egrp = Event::NONE;
    assert( !_filename.empty());
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    if ( !fm->hasMask())
    {
        _egrp |= Event::MESH_CHANGE;
        fm->fixTransformMatrix();
    }   // end if
    //if ( _discardTexture) // Will also need to rebuild the model so don't bother to actually discard the texture
    //  fm->wmesh()->removeAllMaterials();
    const bool wokay = FileIO::FMM::write( fm, &_filename); // Save by specifying new filename
    fm->unlock();
    if ( wokay)
    {
        _egrp |= Event::SAVED_MODEL;
        UndoStates::clear( fm);
    }   // end if
    else
        _egrp |= Event::ERR;
}   // end doAction


Event ActionSaveAsFaceModel::doAfterAction( Event)
{
    if ( has( _egrp, Event::SAVED_MODEL))
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        MS::showStatus( QString("Saved to '%1'").arg(_filename.c_str()), 5000);
    }   // end if
    else
    {
        QString msg( ("Failed saving to '" + _filename + "'!").c_str());
        MS::showStatus( msg, 10000);
        msg.append( ("\n" + FileIO::FMM::error()).c_str());
        QMB::critical( static_cast<QWidget*>(parent()), tr("Failed to save file!"), tr(msg.toStdString().c_str()));
    }   // end else

    _filename = "";
    _discardTexture = false;
    return _egrp;
}   // end doAfterAction

