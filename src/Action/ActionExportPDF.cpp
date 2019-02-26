/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
using FaceTools::Action::U3DCache;
using FaceTools::Action::EventSet;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FaceModel;
using FaceTools::Report::ReportManager;
using FaceTools::Widget::ReportChooserDialog;


// public
ActionExportPDF::ActionExportPDF( const QString& nm, const QIcon& icon, QWidget* p)
    : FaceAction( nm, icon), _dialog( new ReportChooserDialog(p)), _parent(p)
{
    /*
    // Default author info
    if ( !email.isEmpty())
        _author = "\\href{mailto:" + email + "?subject=FaceTools::Action::ActionExportPDF}{Email}";
    */
    //_tmpdir.setAutoRemove(false);    // Uncomment for debug purposes
    assert( _tmpdir.isValid());
    //setAsync(true);
}   // end ctor


bool ActionExportPDF::testReady( const FV*)
{
    //return !fv->data()->landmarks()->empty();
    return true;    // Allow individual reports to enable/disable themselves
}   // end testReady


// All reports assumed available for the selected model currently.
// TODO Set reports to check landmarks on face data.
bool ActionExportPDF::testEnabled( const QPoint*) const
{
    return ready1() && ReportManager::isAvailable();
}   // end testEnabled


// Get the save filepath for the report
bool ActionExportPDF::doBeforeAction( FVS& fvs, const QPoint&)
{
    assert( fvs.size() == 1);
    const FM* fm = fvs.first()->data();

    if ( U3DCache::lock(fm).isEmpty())
        U3DCache::refresh(fm, true);  // Background thread will unlock access to the U3D once written.
    else
        U3DCache::release();

    _report = nullptr;
    _tmpfile = "";
    _err = "";

    if ( _dialog->show(fm))
    {
        const QString rname = _dialog->selectedReportName();
        _report = ReportManager::report(rname);
    }   // end if

    return _report != nullptr;
}   // end doBeforeAction


bool ActionExportPDF::doAction( FVS& fvs, const QPoint&)
{
    assert( fvs.size() == 1);
    _err = "";

    const FM* fm = fvs.first()->data();

    const QString u3dfile = U3DCache::lock( fm);

    _tmpfile = _tmpdir.filePath( "report.pdf");
    const bool genok = _report->generate( fm, u3dfile, _tmpfile);   // May take time - make asynchronous?
    if ( !genok)
        _err = "Failed to generate report PDF!";

    U3DCache::release();

    return genok;
}   // end doAction


void ActionExportPDF::doAfterAction( EventSet& cs, const FVS& fvs, bool v)
{
    if ( !v)
    {
        QMessageBox::warning( _parent, tr("Report Creation Error!"), _err);
        std::cerr << _err.toStdString() << std::endl;
        return;
    }   // end if

    std::cerr << "Created PDF at '" << _tmpfile.toStdString() << "'" << std::endl;

    const FM* fm = fvs.first()->data();
    const std::string fname = FileIO::FMM::filepath( fm);
    QString outfile = boost::filesystem::path(fname).filename().replace_extension( "pdf").string().c_str();

    QFileDialog fileDialog( _parent);
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

        if ( fileNames.empty())
            break;

        outfile = fileNames.first();
        docopy = true;

        if ( QFile::exists(outfile))    // Check if file exists and remove if so
        {
            if ( !QFile::remove(outfile))
            {
                _err = "Unable to remove existing file! Choose a different filename.";
                QMessageBox::warning( _parent, tr("Report Save Error!"), _err);
                std::cerr << _err.toStdString() << std::endl;
                docopy = false; // Try again!
            }   // end if
        }   // end if
    }   // end while

    // Copy the report temporary file to the output location (only succeeds if outfile not present already).
    if ( docopy && QFile::copy( _tmpfile, outfile))
    {
        cs.insert(REPORT_CREATED);
        if ( !_pdfreader.isEmpty())
        {
            std::cerr << "Forking " << _pdfreader.toStdString() << " " << outfile.toStdString() << std::endl;
            QProcess::startDetached(_pdfreader, QStringList(outfile), "");
        }   // end if
    }   // end if
}   // end doAfterAction
