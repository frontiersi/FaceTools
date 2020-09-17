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
using MS = FaceTools::Action::ModelSelector;
using QMB = QMessageBox;

// static members
bool ActionExportPDF::_openOnSave(false);


// public
ActionExportPDF::ActionExportPDF( const QString& nm, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( nm, icon, ks), _dialog( nullptr)
{
    //_tmpdir.setAutoRemove(false);    // Uncomment for debug purposes
    assert( _tmpdir.isValid());
    //setAsync(true);
    addRefreshEvent( Event::CACHE);
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
    fd->setWindowTitle( tr("Save Generated Report"));
    fd->setFileMode( QFileDialog::AnyFile);
    fd->setViewMode( QFileDialog::Detail);
    fd->setNameFilter( "Portable Document Format (*.pdf)");
    fd->setDefaultSuffix( "pdf");
    fd->setAcceptMode( QFileDialog::AcceptSave);
    //fd->setOption( QFileDialog::DontUseNativeDialog);
    return fd;
}   // end createSaveDialog


bool ActionExportPDF::isAvailable( const FM* fm)
{
    return fm && U3DCache::isAvailable() && !U3DCache::u3dfilepath(fm)->isEmpty() && ReportManager::isAvailable();
}   // end isAvailable


bool ActionExportPDF::isAllowed( Event) { return isAvailable(MS::selectedModel());}


// Get the save filepath for the report
bool ActionExportPDF::doBeforeAction( Event)
{
    const FM* fm = MS::selectedModel();
    _report = nullptr;
    _tmpfile = "";
    _err = "";
    if ( _dialog->show(fm))
    {
        const QString rname = _dialog->selectedReportName();
        _report = ReportManager::report(rname);
    }   // end if

    const bool doAct = _report != nullptr;
    if ( doAct)
        MS::showStatus( "Generating report...", 10000);
    return doAct;
}   // end doBeforeAction


void ActionExportPDF::doAction( Event)
{
    _err = "";
    const FM* fm = MS::selectedModel();
    const U3DCache::Filepath u3dfile = U3DCache::u3dfilepath(fm);
    _report->setModelFile( *u3dfile);
    _tmpfile = _tmpdir.filePath( "report.pdf");
    const bool genok = _report->generate( fm, _tmpfile);   // May take time - make asynchronous?
    if ( !genok)
        _err = "Failed to generate report PDF!";
}   // end doAction


Event ActionExportPDF::doAfterAction( Event)
{
    if ( !_err.isEmpty())
    {
        std::cerr << _err.toStdString() << std::endl;
        MS::showStatus("Failed to generate report!", 5000);
        QMB::warning( static_cast<QWidget*>(parent()), tr("Report Creation Error!"), _err);
    }   // end if
    else
        saveGeneratedReport(_tmpfile, _fileDialog);
    return Event::NONE;
}   // end doAfterAction


// static
bool ActionExportPDF::saveGeneratedReport( const QString& tmpfile, QFileDialog *fdialog)
{
    assert( fdialog);
    const FM* fm = MS::selectedModel();
    const QFileInfo outpath( FMM::filepath(fm));

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
                static const QString err = "Unable to overwrite existing file! Is it already open?\nChoose a different filename or close the file.";
                QMB::warning( static_cast<QWidget*>(fdialog->parent()), tr("Report Save Error!"), err);
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
}   // end doAfterAction
