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
#include <QFile>
#include <QMessageBox>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionImportMetaData;
using FaceTools::Action::Event;
using FaceTools::FileIO::FMM;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionImportMetaData::ActionImportMetaData( const QString& dn, const QIcon& ico) : FaceAction(dn, ico), _fdialog(nullptr)
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


QString ActionImportMetaData::_getFileName( const FM* fm)
{
    std::string filename = FMM::filepath( fm);
    boost::filesystem::path inpath( filename);
    filename = inpath.replace_extension( _mimeDB.mimeTypeForName( _fdialog->selectedMimeTypeFilter()).preferredSuffix().toStdString()).string();

    if ( QFile::exists( filename.c_str()))
        _fdialog->selectFile( filename.c_str());

    QString fname;
    if ( _fdialog->exec())
    {
        QStringList fnames = _fdialog->selectedFiles();
        if ( !fnames.empty())
            fname = fnames.first();
    }   // end if

    return fname;
}   // end _getFileName


bool ActionImportMetaData::doBeforeAction( Event)
{
    QWidget* prnt = static_cast<QWidget*>(parent());

    // If the selected model has meta data already, warn about overwriting
    const FM* fm = MS::selectedModel();
    if ( fm->hasMetaData())
    {
        const std::string fname = boost::filesystem::path( FMM::filepath(fm)).filename().string();
        QString msg = tr( ("Model \"" + fname + "\" already has meta data! Overwrite all?").c_str());
        bool doImport = QMessageBox::Yes == QMessageBox::question( prnt, tr("Overwrite existing data?"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if ( !doImport)
            return false;
    }   // end if

    QString fname = _getFileName( fm);
    if ( fname.isEmpty())
        return false;

    _ifs.open( fname.toStdString());
    if ( !_ifs.is_open())
    {
        const QString msg( "Unable to open \'" + fname + "' for reading!");
        QMessageBox::critical( prnt, tr("Import write error!"), msg);
        return false;
    }   // end if

    _filename = fname;
    return true;
}   // end doBeforeAction


void ActionImportMetaData::doAction( Event)
{
    storeUndo( this, Event::ASSESSMENT_CHANGE | Event::LANDMARKS_CHANGE | Event::PATHS_CHANGE);

    QMimeType mtype = _mimeDB.mimeTypeForFile(QFileInfo(_filename));
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

    std::string unused;
    double fversion;
    FileIO::importMetaData( *fm, tree, fversion, unused);
    fm->moveToSurface();
    fm->unlock();

    _ifs.close();
}   // end doAction


Event ActionImportMetaData::doAfterAction( Event)
{
    MS::showStatus( QString("Imported meta data from '%1'.").arg(_filename), 5000);
    return Event::ASSESSMENT_CHANGE | Event::LANDMARKS_CHANGE | Event::PATHS_CHANGE;
}   // end doAfterAction

