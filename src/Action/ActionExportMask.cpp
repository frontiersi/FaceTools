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

#include <Action/ActionExportMask.h>
#include <FileIO/FaceModelManager.h>
#include <r3dio/IOHelpers.h>
#include <QMessageBox>
#include <QFileInfo>
#include <FaceModel.h>
#include <FaceTools.h>
#include <cassert>
using FaceTools::Action::ActionExportMask;
using FaceTools::Action::FaceAction;
using FaceTools::Action::UndoState;
using FaceTools::Action::Event;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionExportMask::ActionExportMask( const QString& dn, const QIcon& ico, const QKeySequence &ks)
    : FaceAction(dn, ico, ks), _fdialog(nullptr), _filename(""), _savedOkay(false)
{
    addRefreshEvent( Event::MASK_CHANGE);
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
    FM::RPtr fm = MS::selectedModelScopedRead();
    return fm && fm->hasMask();
}   // end isAllowedd


bool ActionExportMask::doBeforeAction( Event)
{
    QStringSet exts = FMM::fileFormats().exportExtensions();
    exts.erase( FMM::fileFormats().preferredExt());
    _fdialog->setNameFilters( FMM::fileFormats().createExportFilters( exts));

    const QFileInfo origfile( FMM::filepath( *MS::selectedModelScopedRead()));
    const QString outpath = origfile.path() + "/" + origfile.baseName() + "_am_corr.ply";

    _fdialog->setDirectory( origfile.path());
    _fdialog->selectFile( outpath);

    QWidget *prnt = static_cast<QWidget*>(parent());
    while ( _filename.isEmpty())
    {
        QStringList fnames;
        if ( _fdialog->exec())
            fnames = _fdialog->selectedFiles();

        _filename = fnames.empty() ? "" : fnames.first().trimmed();
        if ( _filename.isEmpty())
            break;

        const QFileInfo fpath(_filename);
        const QString ext = fpath.suffix();
        if ( !ext.isEmpty())
        {
            if ( exts.count(ext) == 0)
            {
                static const QString msg = tr("Invalid format for mesh export; choose from one of those listed.");
                QMB::information( prnt, tr("Invalid File Format"), QString("<p align='center'>%1</p>").arg(msg));
                _filename = "";
                continue;
            }   // end if
        }   // end if
        else
        {
            const QString cext = FMM::fileFormats().extensionForFilter( _fdialog->selectedNameFilter());
            _filename = fpath.path() + "/" + fpath.baseName() + "." + cext;
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

    if ( !_filename.isEmpty())
        MS::showStatus( QString( "Saving mask to '%1'...").arg(_filename));

    _savedOkay = false;
    return !_filename.isEmpty();
}   // end doBeforeAction


void ActionExportMask::doAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    _savedOkay = r3dio::saveMesh( fm->mask(), _filename.toLocal8Bit().toStdString());
}   // end doAction


Event ActionExportMask::doAfterAction( Event)
{
    if ( _savedOkay)
        MS::showStatus( QString("Saved mask to '%1'").arg(_filename), 5000);
    else
    {
        const QString msg = tr("Failed to export mask to '%1'!").arg(_filename);
        MS::showStatus( msg, 10000);
        QMB::critical( static_cast<QWidget*>(parent()), tr("Export Error!"),
                        QString("<p align='center'>%1</p>").arg(msg));
    }   // end else

    _filename = "";
    _savedOkay = false;
    return Event::NONE;
}   // end doAfterAction
