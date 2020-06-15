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

#include <Action/ActionImportMetaData.h>
#include <FileIO/FaceModelManager.h>
#include <FileIO/FaceModelXMLFileHandler.h>
#include <FaceModel.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionImportMetaData;
using FaceTools::Action::Event;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::Action::ModelSelector;
using QMB = QMessageBox;


ActionImportMetaData::ActionImportMetaData( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _fdialog(nullptr), _err(false)
{
    _mimefilters << "text/xml" << "application/json";
}   // end ctor


void ActionImportMetaData::postInit()
{
    _fdialog = new QFileDialog( static_cast<QWidget*>(parent()));
    _fdialog->setWindowTitle( QObject::tr("Import metadata from..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setMimeTypeFilters( _mimefilters);
    _fdialog->setDefaultSuffix( _mimefilters.front());
    _fdialog->setAcceptMode( QFileDialog::AcceptOpen);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
}   // end postInit


QString ActionImportMetaData::toolTip() const
{
    return "Import metadata from a JSON or XML file and set these data (possibly overwriting) in the selected model.";
}   // end toolTip


bool ActionImportMetaData::isAllowed( Event) { return MS::isViewSelected();}


QString ActionImportMetaData::_getFilePath( const FM* fm)
{
    const auto &filter = _fdialog->selectedMimeTypeFilter();
    const QFileInfo fpath( FMM::filepath(fm));
    QString fp = fpath.path() + "/" + fpath.baseName() + "." + _mimeDB.mimeTypeForName( filter).preferredSuffix();
    if ( QFile::exists( fp))
        _fdialog->selectFile( fp);

    fp = "";
    if ( _fdialog->exec())
    {
        QStringList fnames = _fdialog->selectedFiles();
        if ( !fnames.empty())
            fp = fnames.first();
    }   // end if

    return fp;
}   // end _getFilePath


bool ActionImportMetaData::doBeforeAction( Event)
{
    QWidget* prnt = static_cast<QWidget*>(parent());

    // If the selected model has meta data already, warn about overwriting
    const FM* fm = MS::selectedModel();
    if ( fm->hasMetaData())
    {
        const QString fname = QFileInfo( FMM::filepath(fm)).fileName();
        const QString msg = tr( ("Model \"" + fname.toLocal8Bit().toStdString() + "\" already has meta data! Overwrite all?").c_str());
        bool doImport = QMB::Yes == QMB::question( prnt, tr("Overwrite existing data?"), msg, QMB::Yes | QMB::No, QMB::No);
        if ( !doImport)
            return false;
    }   // end if

    const QString fp = _getFilePath( fm);
    if ( fp.isEmpty())
        return false;

    _ifs.open( fp.toLocal8Bit().toStdString());
    if ( !_ifs.is_open())
    {
        const QString msg = tr( ("Unable to open \'" + fp.toLocal8Bit().toStdString() + "' for reading!").c_str());
        QMB::critical( prnt, tr("Import write error!"), msg);
        return false;
    }   // end if

    _err = false;
    _filepath = fp;
    return true;
}   // end doBeforeAction


void ActionImportMetaData::doAction( Event)
{
    storeUndo( this, Event::ASSESSMENT_CHANGE | Event::LANDMARKS_CHANGE | Event::PATHS_CHANGE);

    QMimeType mtype = _mimeDB.mimeTypeForFile(QFileInfo(_filepath));
    assert(mtype.isValid());

    PTree tree;
    for ( const QString& nm : _mimefilters)
    {
        if ( mtype.inherits( nm))
        {
            if ( nm == "text/xml")
                boost::property_tree::read_xml( _ifs, tree);
            else if ( nm == "application/json")
                boost::property_tree::read_json( _ifs, tree);
            else
                assert(false);
            break;
        }   // end if
    }   // end for

    FM* fm = MS::selectedModel();
    fm->lockForWrite();

    _err = false;
    double fversion;
    if ( FileIO::importMetaData( *fm, tree, fversion))
    {
        fm->moveToSurface();
        fm->unlock();
    }   // end if
    else
    {
        _err = true;
        scrapLastUndo( fm);
    }   // end else

    _ifs.close();
}   // end doAction


Event ActionImportMetaData::doAfterAction( Event)
{
    Event ev = Event::NONE;
    if ( _err)
        MS::showStatus( QString("Unable to import data from '%1'!").arg(_filepath), 10000);
    else
    {
        MS::showStatus( QString("Imported meta data from '%1'.").arg(_filepath), 5000);
        ev = Event::ASSESSMENT_CHANGE | Event::LANDMARKS_CHANGE | Event::PATHS_CHANGE;
    }   // end else
    return ev;
}   // end doAfterAction

