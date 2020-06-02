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

#include <Action/ActionExportMask.h>
#include <FileIO/FaceModelManager.h>
#include <boost/filesystem.hpp>
#include <r3dio/IOHelpers.h>
#include <QMessageBox>
#include <FaceModel.h>
#include <FaceTools.h>
#include <rlib/FileIO.h>
#include <cassert>
using FaceTools::Action::ActionExportMask;
using FaceTools::Action::FaceAction;
using FaceTools::Action::UndoState;
using FaceTools::Action::Event;
using FaceTools::FileIO::FMM;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;
using QMB = QMessageBox;
namespace BFS = boost::filesystem;


ActionExportMask::ActionExportMask( const QString& dn, const QIcon& ico, const QKeySequence &ks)
    : FaceAction(dn, ico, ks), _fdialog(nullptr), _filename(""), _savedOkay(false)
{
    setAsync(true);
}   // end ctor


void ActionExportMask::postInit()
{
    _fdialog = new QFileDialog( static_cast<QWidget*>(parent()));
    _fdialog->setWindowTitle( tr("Export mask as..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setAcceptMode( QFileDialog::AcceptSave);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
}   // end postInit


QString ActionExportMask::whatsThis() const
{
    QStringList htext;
    htext << "Save the selected model's mask in a standalone file.";
    htext << "No metadata or any other information about the selected model is saved!";
    return htext.join(" ");
}   // end whatsThis


bool ActionExportMask::isAllowed( Event)
{
    const FM *fm = MS::selectedModel();
    return fm && fm->hasMask();
}   // end isAllowedd


bool ActionExportMask::doBeforeAction( Event)
{
    QStringSet exts = FMM::fileFormats().exportExtensions();
    exts.erase( FMM::fileFormats().preferredExt());
    _fdialog->setNameFilters( FMM::fileFormats().createExportFilters( exts));

    const BFS::path origfile( FMM::filepath(MS::selectedModel()));
    BFS::path outpath = origfile.parent_path() / (origfile.stem().string() + "_am_corr.ply");

    _fdialog->setDirectory( outpath.parent_path().string().c_str());
    _fdialog->selectFile( outpath.string().c_str());

    QWidget *prnt = static_cast<QWidget*>(parent());
    while ( _filename.empty())
    {
        QStringList fnames;
        if ( _fdialog->exec())
            fnames = _fdialog->selectedFiles();

        _filename = fnames.empty() ? "" : fnames.first().trimmed().toStdString();
        if ( _filename.empty())
            break;

        BFS::path fpath(_filename);
        if ( fpath.has_extension())
        {
            const QString ext = rlib::getExtension( _filename).c_str();
            if ( exts.count(ext) == 0)
            {
                static const QString msg = tr("Invalid format for mesh export; choose from one of those listed.");
                QMB::information( prnt, tr("Invalid File Format"), msg);
                _filename = "";
                continue;
            }   // end if
        }   // end if
        else
        {
            const QString cext = FMM::fileFormats().extensionForFilter( _fdialog->selectedNameFilter());
            _filename = fpath.replace_extension( cext.toStdString()).string();
        }   // end else

        /*
        if ( !FMM::canSaveTextures(_filename))
        {
            static const QString tit = tr("Discard Texture?!");
            static const QString msg = tr("Texture will not be saved in this format! Continue?");
            if ( QMB::No == QMB::question( prnt, tit, msg, QMB::Yes | QMB::No, QMB::Yes))
                _filename = "";
        }   // end if
        */
    }   // end while

    if ( !_filename.empty())
        MS::showStatus( QString( "Saving mask to '%1'...").arg(_filename.c_str()));

    _savedOkay = false;
    return !_filename.empty();
}   // end doBeforeAction


void ActionExportMask::doAction( Event)
{
    const FM *fm = MS::selectedModel();
    assert( !_filename.empty());
    fm->lockForRead();
    _savedOkay = r3dio::saveMesh( fm->mask(), _filename);
    fm->unlock();
}   // end doAction


Event ActionExportMask::doAfterAction( Event)
{
    if ( _savedOkay)
        MS::showStatus( QString("Saved mask to '%1'").arg(_filename.c_str()), 5000);
    else
    {
        const QString msg = tr("Failed saving mask to '%1'!").arg(_filename.c_str());
        MS::showStatus( msg, 10000);
        QMB::critical( static_cast<QWidget*>(parent()), tr("Failed to save mesh!"), msg);
    }   // end else

    _filename = "";
    _savedOkay = false;
    return Event::NONE;
}   // end doAfterAction
