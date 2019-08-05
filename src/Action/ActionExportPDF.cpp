/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionExportPDF.h>
#include <U3DCache.h>
#include <ReportManager.h>
#include <FaceModelManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>

#include <QTemporaryDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFile>

#include <boost/filesystem.hpp>
#include <fstream>
#include <cassert>
using FaceTools::Action::ActionExportPDF;
using FaceTools::Action::Event;
using FaceTools::Report::ReportManager;
using FaceTools::Widget::ReportChooserDialog;
using FaceTools::U3DCache;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;

QString ActionExportPDF::_pdfreader("");    // Static

// public
ActionExportPDF::ActionExportPDF( const QString& nm, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( nm, icon, ks), _dialog( nullptr)
{
    //_tmpdir.setAutoRemove(false);    // Uncomment for debug purposes
    assert( _tmpdir.isValid());
    //setAsync(true);
}   // end ctor


void ActionExportPDF::postInit()
{
    _dialog = new ReportChooserDialog( static_cast<QWidget*>(parent()));
}   // end postInit


bool ActionExportPDF::isAvailable( const FM* fm)
{
    return fm && U3DCache::isAvailable() && !U3DCache::u3dfilepath(fm)->isEmpty() && ReportManager::isAvailable();
}   // end isAvailable


bool ActionExportPDF::checkEnable( Event) { return isAvailable(MS::selectedModel());}


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
        MS::showStatus( "Generating report...");
    return doAct;
}   // end doBeforeAction


void ActionExportPDF::doAction( Event)
{
    _err = "";
    const FM* fm = MS::selectedModel();
    const U3DCache::Filepath u3dfile = U3DCache::u3dfilepath(fm);
    _tmpfile = _tmpdir.filePath( "report.pdf");
    const bool genok = _report->generate( fm, *u3dfile, _tmpfile);   // May take time - make asynchronous?
    if ( !genok)
        _err = "Failed to generate report PDF!";
}   // end doAction


void ActionExportPDF::doAfterAction( Event)
{
    QWidget* prnt = static_cast<QWidget*>(parent());

    if ( !_err.isEmpty())
    {
        QMessageBox::warning( prnt, tr("Report Creation Error!"), _err);
        MS::showStatus("Failed to generate report!", 5000);
        std::cerr << _err.toStdString() << std::endl;
        return;
    }   // end if

    std::cerr << "Created PDF at '" << _tmpfile.toStdString() << "'" << std::endl;
    saveGeneratedReport(_tmpfile, prnt);
}   // end doAfterAction


// static
bool ActionExportPDF::saveGeneratedReport( const QString& tmpfile, const QWidget* prnt)
{
    const FM* fm = MS::selectedModel();
    const std::string fname = FileIO::FMM::filepath( fm);
    QString outfile = boost::filesystem::path(fname).filename().replace_extension( "pdf").string().c_str();

    QFileDialog fileDialog( const_cast<QWidget*>(prnt));
    fileDialog.setWindowTitle( tr("Save Generated Report"));
    fileDialog.setFileMode( QFileDialog::AnyFile);
    fileDialog.setNameFilter( "Portable Document Format (*.pdf)");
    fileDialog.setDefaultSuffix( "pdf");
    fileDialog.selectFile( outfile);
    fileDialog.setAcceptMode( QFileDialog::AcceptSave);
    fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    bool docopy = false;
    while ( !docopy)
    {
        QStringList fileNames;
        if ( fileDialog.exec() > 0)
            fileNames = fileDialog.selectedFiles();

        outfile = "";
        if ( fileNames.empty())
            break;

        outfile = fileNames.first();
        docopy = true;

        if ( QFile::exists(outfile))    // Check if file exists and remove if so
        {
            if ( !QFile::remove(outfile))
            {
                static const QString err = "Unable to remove existing file! Choose a different filename.";
                QMessageBox::warning( const_cast<QWidget*>(prnt), tr("Report Save Error!"), err);
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
        if ( !_pdfreader.isEmpty())
        {
            std::cerr << "Forking " << _pdfreader.toStdString() << " " << outfile.toStdString() << std::endl;
            QProcess::startDetached(_pdfreader, QStringList(outfile), "");
        }   // end if
    }   // end if

    return success;
}   // end doAfterAction
