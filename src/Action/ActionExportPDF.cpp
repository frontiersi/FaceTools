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

#include <Action/ActionExportPDF.h>
#include <U3DCache.h>
#include <Report/ReportManager.h>
#include <FileIO/FaceModelManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>

#include <QDesktopServices>
#include <QTemporaryDir>
#include <QMessageBox>
#include <QFile>

#include <fstream>
#include <cassert>
using FaceTools::Action::ActionExportPDF;
using FaceTools::Action::Event;
using FaceTools::Report::ReportManager;
using FaceTools::Widget::ReportChooserDialog;
using FaceTools::U3DCache;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;

// static members
bool ActionExportPDF::_openOnSave(false);


// public
ActionExportPDF::ActionExportPDF( const QString& nm, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( nm, icon, ks), _dialog( nullptr)
{
    //_tmpdir.setAutoRemove(false);    // Uncomment for debug purposes
    assert( _tmpdir.isValid());
    addRefreshEvent( Event::CACHE);
    //setAsync(true);
}   // end ctor


void ActionExportPDF::postInit()
{
    QWidget *prnt = static_cast<QWidget*>(parent());
    _dialog = new ReportChooserDialog( prnt);
    _fileDialog = createSaveDialog( prnt);
}   // end postInit


// static
QFileDialog* ActionExportPDF::createSaveDialog( QWidget *prnt)
{
    QFileDialog *fd = new QFileDialog( prnt);
    fd->setWindowTitle( tr("Save Report"));
    fd->setFileMode( QFileDialog::AnyFile);
    fd->setViewMode( QFileDialog::Detail);
    fd->setNameFilter( "Portable Document Format (*.pdf)");
    fd->setDefaultSuffix( "pdf");
    fd->setAcceptMode( QFileDialog::AcceptSave);
    //fd->setOption( QFileDialog::DontUseNativeDialog);
    return fd;
}   // end createSaveDialog


bool ActionExportPDF::isAvailable()
{
    if ( !ReportManager::isAvailable() || !U3DCache::isAvailable())
        return false;

    const FM *fm0 = MS::selectedModel();
    const FM *fm1 = MS::nonSelectedModel();
    const bool v = fm0 && !U3DCache::u3dfilepath(*fm0)->isEmpty();
    return v && (!fm1 || !U3DCache::u3dfilepath(*fm1)->isEmpty());
}   // end isAvailable


bool ActionExportPDF::isAllowed( Event) { return isAvailable();}


// Get the save filepath for the report
bool ActionExportPDF::doBeforeAction( Event)
{
    _report = nullptr;
    _tmpfile = "";
    _err = "";
    if ( _dialog->show())
        _report = ReportManager::report(_dialog->selectedReportName());
    const bool doAct = _report != nullptr;
    if ( doAct)
        MS::showStatus( "Generating report...", 10000);
    return doAct;
}   // end doBeforeAction


void ActionExportPDF::doAction( Event)
{
    _err = "";
    _tmpfile = _tmpdir.filePath( "report.pdf");
    const bool genok = _report->generate( _tmpfile);
    if ( !genok)
        _err = _report->errorMsg();
}   // end doAction


Event ActionExportPDF::doAfterAction( Event)
{
    if ( !_err.isEmpty())
    {
        std::cerr << _err.toStdString() << std::endl;
        MS::showStatus("Failed to generate report!", 5000);
        QMB::warning( static_cast<QWidget*>(parent()), tr("Report Creation Error!"),
                      QString("<p align='center'>%1</p>").arg(_err));
    }   // end if
    else
        saveGeneratedReport(_tmpfile, _fileDialog);
    return Event::NONE;
}   // end doAfterAction


// static
bool ActionExportPDF::saveGeneratedReport( const QString& tmpfile, QFileDialog *fdialog)
{
    assert( fdialog);
    const FM *fm = MS::selectedModel();
    const QFileInfo outpath( FMM::filepath(*fm));

    fdialog->setDirectory( outpath.path());
    QString outfile = outpath.path() + "/" + outpath.baseName() + ".pdf";
    fdialog->selectFile( outfile);

    bool docopy = false;
    while ( !docopy)
    {
        QStringList fileNames;
        if ( fdialog->exec() > 0)
            fileNames = fdialog->selectedFiles();

        outfile = "";
        if ( fileNames.empty())
        {
            MS::showStatus( "Report discarded!", 5000);
            break;
        }   // end if

        outfile = fileNames.first();
        docopy = true;

        if ( QFile::exists(outfile))    // Check if file exists and remove if so
        {
            if ( !QFile::remove(outfile))
            {
                static const QString err = tr("Unable to overwrite existing file! Is it already open?<br>Choose a different filename or close the file.");
                QMB::warning( static_cast<QWidget*>(fdialog->parent()), tr("Report Save Error!"),
                              QString("<p align='center'>%1</p>").arg(err));
                docopy = false; // Try again!
            }   // end if
        }   // end if
    }   // end while

    bool success = false;
    // Copy the report temporary file to the output location (only succeeds if outfile not present already).
    if ( docopy && QFile::copy( tmpfile, outfile))
    {
        MS::showStatus( "Report saved to '" + outfile + "'", 5000);
        success = true;
        if ( _openOnSave)
        {
            //std::cerr << "Forking " << _pdfreader.toStdString() << " " << outfile.toStdString() << std::endl;
            //QProcess::startDetached(_pdfreader, QStringList(outfile), "");
            QDesktopServices::openUrl( QUrl( "file:///" + outfile, QUrl::TolerantMode));
        }   // end if
    }   // end if

    return success;
}   // end saveGeneratedReport
