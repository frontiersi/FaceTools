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

#include <Action/ActionExportMetaData.h>
#include <FileIO/FaceModelManager.h>
#include <FileIO/FaceModelFileData.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <QSet>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cassert>
using FaceTools::Action::ActionExportMetaData;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionExportMetaData::ActionExportMetaData( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _fdialog(nullptr)
{
    _mimefilters << "text/csv" << "application/json" << "text/xml";
}   // end ctor


void ActionExportMetaData::postInit()
{
    _fdialog = new QFileDialog( static_cast<QWidget*>(parent()));
    _fdialog->setWindowTitle( QObject::tr("Export metadata to..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setAcceptMode( QFileDialog::AcceptSave);
    _fdialog->setMimeTypeFilters( _mimefilters);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
}   // end postInit


QString ActionExportMetaData::toolTip() const
{
    return "Export metadata about the selected model to CSV, JSON, or XML format.";
}   // end toolTip


bool ActionExportMetaData::isAllowed( Event) { return MS::isViewSelected();}


QString ActionExportMetaData::_createFilePath( const QString &fname) const
{
    QFileInfo fpath( fname);
    fpath.makeAbsolute();
    const QString filter = _fdialog->selectedMimeTypeFilter();
    QString suffix = _mimeDB.mimeTypeForName( filter).preferredSuffix().trimmed();
    if ( suffix.isEmpty())  // For some reason, on Windows this is empty on the first call!
        suffix = "csv";
    return fpath.absolutePath() + "/" + fpath.completeBaseName() + "." + suffix;
}   // end _createFilePath


QString ActionExportMetaData::_createFilePath( const FM &fm) const
{
    return _createFilePath( FMM::filepath( fm));
}   // end _createFilePath


QString ActionExportMetaData::_getFilePath( const FM &fm)
{
    QString fp = _createFilePath(fm);

    QString fname;
    while ( fname.isEmpty())
    {
        const QString cpath = QFileInfo(fp).absolutePath();
        _fdialog->setDirectory( cpath);
        _fdialog->selectFile( fp);
        QStringList fnames;
        if ( _fdialog->exec())
            fnames = _fdialog->selectedFiles();
        if ( fnames.empty())
            break;

        fname = fnames.first();
        bool isValidMime = false;
        const QFileInfo finfo(fname);
        QMimeType mtype = _mimeDB.mimeTypeForFile( finfo);
        if ( mtype.isValid())
        {
            for ( const QString& mn : _mimefilters)
            {
                if ( mtype.inherits( mn))
                {
                    isValidMime = true;
                    break;
                }   // end if
            }   // end for
        }   // end if

        if ( !isValidMime)
        {
            fname = "";
            fp = _createFilePath( finfo.filePath());
        }   // end if
    }   // end while

    return fname;
}   // end _getFilePath


bool ActionExportMetaData::doBeforeAction( Event)
{
    const QString fp = _getFilePath( *MS::selectedModelScopedRead());
    if ( fp.isEmpty())
        return false;

    _ofs.open( fp.toStdString());
    if ( !_ofs.is_open())
    {
        const QString msg = tr( ("Cannot open \'" + fp.toStdString() + "' for writing!").c_str());
        QWidget* prnt = static_cast<QWidget*>(parent());
        QMB::critical( prnt, tr("Export Write Error!"), QString("<p align='center'>%1</p>").arg(msg));
        return false;
    }   // end if

    _filepath = fp;
    return true;
}   // end doBeforeAction


void ActionExportMetaData::doAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    PTree tree;
    FileIO::exportMetaData( *fm, true/* export path metrics*/, tree);

    QMimeType mtype = _mimeDB.mimeTypeForFile( QFileInfo(_filepath));
    assert(mtype.isValid());

    for ( const QString& nm : _mimefilters)
    {
        if ( mtype.inherits( nm))
        {
            if ( nm == "text/csv")
                FileIO::FaceModelFileData( *fm).toCSV( _ofs);
            else if ( nm == "text/xml")
                boost::property_tree::write_xml( _ofs, tree);
            else if ( nm == "application/json")
                boost::property_tree::write_json( _ofs, tree);
            else
                assert(false);
            break;
        }   // end if
    }   // end for

    _ofs.close();
}   // end doAction


Event ActionExportMetaData::doAfterAction( Event)
{
    MS::showStatus( QString("Exported metadata to '%1'.").arg(_filepath), 5000);
    return Event::NONE;
}   // end doAfterAction
