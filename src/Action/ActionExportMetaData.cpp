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

#include <Action/ActionExportMetaData.h>
#include <FileIO/FaceModelManager.h>
#include <FileIO/FaceModelXMLFileHandler.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <QSet>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionExportMetaData;
using FaceTools::Action::Event;
using FaceTools::FileIO::FMM;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionExportMetaData::ActionExportMetaData( const QString& dn, const QIcon& ico) : FaceAction(dn, ico), _fdialog(nullptr)
{
    _mimefilters << "text/xml" << "application/json";
}   // end ctor


void ActionExportMetaData::postInit()
{
    _fdialog = new QFileDialog( static_cast<QWidget*>(parent()));
    _fdialog->setWindowTitle( QObject::tr("Export metadata to..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setAcceptMode( QFileDialog::AcceptSave);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
}   // end postInit


QString ActionExportMetaData::toolTip() const
{
    return "Export metadata about the selected model to JSON or XML file format.";
}   // end toolTip


bool ActionExportMetaData::isAllowed( Event) { return MS::isViewSelected();}


QString ActionExportMetaData::_getFileName( const FM* fm)
{
    _fdialog->setMimeTypeFilters( _mimefilters);

    std::string filename = FMM::filepath( fm);
    boost::filesystem::path outpath( filename);
    filename = outpath.replace_extension( _mimeDB.mimeTypeForName( _fdialog->selectedMimeTypeFilter()).preferredSuffix().toStdString()).string();

    QString fname;
    while ( fname.isEmpty())
    {
        _fdialog->selectFile( filename.c_str());
        QStringList fnames;
        if ( _fdialog->exec())
            fnames = _fdialog->selectedFiles();

        fname = "";
        if ( !fnames.empty())
            fname = fnames.first();
        else
            break;

        bool isValidMime = false;
        QMimeType mtype = _mimeDB.mimeTypeForFile(QFileInfo(fname));
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
            boost::filesystem::path fpath( fname.toStdString());
            fname = fpath.replace_extension( _mimeDB.mimeTypeForName( _fdialog->selectedMimeTypeFilter()).preferredSuffix().toStdString()).string().c_str();
        }   // end if
    }   // end while

    return fname;
}   // end _getFileName


bool ActionExportMetaData::doBeforeAction( Event)
{
    QString fname = _getFileName( MS::selectedModel());
    if ( fname.isEmpty())
        return false;

    _ofs.open( fname.toStdString());
    if ( !_ofs.is_open())
    {
        const QString msg( "Unable to open \'" + fname + "' for writing!");
        QWidget* prnt = static_cast<QWidget*>(parent());
        QMessageBox::critical( prnt, tr("Export write error!"), msg);
        return false;
    }   // end if

    _filename = fname;
    return true;
}   // end doBeforeAction


void ActionExportMetaData::doAction( Event)
{
    FM* fm = MS::selectedModel();
    fm->lockForRead();
    PTree tree;
    FileIO::exportMetaData( fm, FMM::filepath(fm), true/* export path metrics*/, tree);
    fm->unlock();

    QMimeType mtype = _mimeDB.mimeTypeForFile(QFileInfo(_filename));
    assert(mtype.isValid());

    for ( const QString& nm : _mimefilters)
    {
        if ( mtype.inherits( nm))
        {
            if ( nm == "text/xml")
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
    MS::showStatus( QString("Exported meta data to '%1'.").arg(_filename), 5000);
    return Event::NONE;
}   // end doAfterAction
