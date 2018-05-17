/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#include <ModelViewer.h>
#include <FaceControlSet.h>
#include <FaceControl.h>
#include <FaceModel.h>

#include <PDFGenerator.h>      // RModelIO
#include <U3DExporter.h>       // RModelIO

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>
#include <cassert>
using FaceTools::ActionExportPDF;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::ModelViewer;
using RModelIO::PDFGenerator;
using RModelIO::U3DExporter;
using RFeatures::CameraParams;
using RFeatures::ObjModel;


namespace {

bool writeDoc( PDFGenerator::LaTeXU3DInserter* fig, const std::string& texfile, const std::string& logofile)
{
    assert(fig);
    bool writtenOkay = false;
    std::ofstream os;
    try
    {
        os.open( texfile.c_str(), std::ios::out);
        os << "\\documentclass{article}" << std::endl;
        os << "\\usepackage[a4paper]{geometry}" << std::endl;
        os << "\\usepackage{a4wide}" << std::endl;
        os << "\\usepackage{media9}" << std::endl;
        os << "\\usepackage[parfill]{parskip}" << std::endl;
        os << "\\usepackage[colorlinks=true,urlcolor=red]{hyperref}" << std::endl;
        os << "\\usepackage{graphicx}" << std::endl;
        os << "\\DeclareGraphicsExtensions{.png,.jpg,.pdf}" << std::endl;
        os << std::endl;
        os << "\\begin{document}" << std::endl;
        os << "\\title{3D-FAST: Facial Analysis Report (Beta)}" << std::endl;
        os << "\\author{\\href{mailto:r.l.palmer@curtin.edu.au?subject=3D-FAST\\%20PDF\\%20report}{Email}}" << std::endl;
        os << "\\maketitle" << std::endl;
        os << "\\thispagestyle{empty}" << std::endl;
        os << std::endl;

        os << *fig << std::endl;

        if ( !logofile.empty())
        {
            os << "\\begin{figure}[!ht]" << std::endl;
            os << "\\centering" << std::endl;
            os << "\\includegraphics[]{" << logofile << "}" << std::endl;
            os << "\\end{figure}" << std::endl;
        }   // end if

        os << "\\end{document}" << std::endl;
        os.flush();
        writtenOkay = true;
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "[EXCEPTION!] Exception writing to file " << texfile << std::endl;
        std::cerr << e.what() << std::endl;
        writtenOkay = false;
    }   // end catch
    os.close();
    return writtenOkay;
}   // end writeDoc

}   // end namespace


// public
ActionExportPDF::ActionExportPDF( BaseReportTemplate* t, QWidget* p, QProgressBar* pb)
    : FaceAction( t->getDisplayName(), t->getIcon() == NULL ? QIcon(), t-getIcon()),
      _template(t), _parent(p),
      _workdir( boost::filesystem::path( qApp->applicationDirPath().toStdString()))
{
    assert(t);
    PDFGenerator::pdflatex = PDF_LATEX;
    U3DExporter::IDTFConverter = IDTF_CONVERTER;
    std::cerr << "IDTFConverter : " << U3DExporter::IDTFConverter << " ("
              << (U3DExporter::isAvailable()  ? "" : "not ") << "available)" << std::endl;
    std::cerr << "pdflatex      : " << PDFGenerator::pdflatex << " ("
              << (PDFGenerator::isAvailable() ? "" : "not ") << "available)" << std::endl;

    // Set the working directory for latex output.
    boost::filesystem::path wdir = boost::filesystem::path( LATEX_WORK);
    assert( boost::filesystem::is_directory( wdir));
    _workdir = wdir;

    // Set the logo to use
    _logopath = boost::filesystem::path( LATEX_LOGO);
    assert( boost::filesystem::exists( _logopath));

    _template->setAction(this); // Allows report template to add extra respond to events for checking availability.
    if ( pb)
        setAsync( true, QTools::QProgressUpdater::create(pb));
}   // end ctor


// public
bool ActionExportPDF::testReady( FaceControl* fc)
{
    // Could also test in here for other FaceControls on the same viewer - if the report needed them for example.
    return _template->isAvailable(fc->data());
}   // end testReady


bool ActionExportPDF::testEnabled()
{
    return PDFGenerator::isAvailable() && U3DExporter::isAvailable();
}   // end testEnabled

// TODO

// protected virtual
bool ActionExportPDF::doBeforeAction( FaceControlSet& fcs)
{
    assert( _fconts.size() == 1);
    const FaceControl* fcont = *_fconts.begin();
    assert( fcont->getViewer());
    std::string outfile = fcont->getModel()->getObjectMeta()->getObjectFile();
    boost::filesystem::path outpath(outfile);
    const QString parentDir = outpath.parent_path().string().c_str();

    // Make _outfile have the preferred extension
    outfile = outpath.replace_extension( "pdf").string();

    QFileDialog fileDialog;
    fileDialog.setWindowTitle( tr("Export as PDF"));
    fileDialog.setFileMode( QFileDialog::AnyFile);
    fileDialog.setNameFilter( "Portable Document Format (*.pdf)");
    fileDialog.setDirectory( parentDir);    // Default save directory is last save location for model
    fileDialog.setDefaultSuffix( "pdf");
    fileDialog.selectFile( outfile.c_str());
    fileDialog.setAcceptMode( QFileDialog::AcceptSave);
    fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    QStringList fileNames;
    if ( fileDialog.exec())
        fileNames = fileDialog.selectedFiles();
    fileDialog.close();

    _pdffile = "";
    if ( !fileNames.empty())
        _pdffile = fileNames.first().toStdString();
    return !_pdffile.empty();
}   // end doBeforeAction


// protected virtual
void ActionExportPDF::doAfterAction( bool v)
{
    if ( !v)
        QMessageBox::warning( _fapp, tr("PDF Export Error!"), tr( _err.c_str()));

    if ( !_pdffile.empty())
    {
        std::cerr << "Created PDF at '" << _pdffile << "'" << std::endl;
        QMessageBox::information( _fapp, tr("Created PDF"),
                                  tr( (std::string("Created PDF at '") + _pdffile + "'").c_str()));
    }   // end if
    _pupdater.reset();
    _pdffile = "";
}   // end doAfterAction


// public
bool ActionExportPDF::doAction()
{
    if ( _pdffile.empty())
        return true;

    std::cerr << "[INFO] FaceApp::ActionExportPDF::doAction: Commencing PDF generation..." << std::endl;
    assert( _fconts.size() == 1);
    const FaceControl* fcont = *_fconts.begin();

    _pupdater.reset();
    _pupdater.processUpdate(0.01f);

    const std::string texpath = (_workdir / "main.tex").string();
    _err = "";

    // Currently, the model itself is written out, not the VTK view (next version!)
    const ModelViewer* viewer = fcont->getViewer();
    const ObjModel::Ptr model = fcont->getModel()->getObjectMeta()->getObject();
    PDFGenerator pdfgen;
    _pupdater.processUpdate(0.2f);
    PDFGenerator::LaTeXU3DInserter* fig = pdfgen.getFigureInserter( model, 150, 150, viewer->getCamera(), "3D Facial Image (Beta)");
    if ( !fig)
        _err = "Failed to create U3D model from data!";
    _pupdater.processUpdate(0.6f);

    if ( _err.empty())
    {
        if ( !writeDoc( fig, texpath, _logopath.string()))
            _err = "Failed to create '" + texpath + "' for LaTeX parsing!";
    }   // end if
    _pupdater.processUpdate(0.65f);

    if ( _err.empty())
    {
        if ( !pdfgen( texpath, true))
            _err = "Failed to generate PDF!";
    }   // end if
    _pupdater.processUpdate(0.9f);

    const boost::filesystem::path pdfout( _workdir / "main.pdf");
    if ( _err.empty() && boost::filesystem::exists( pdfout))
    {
        boost::system::error_code errCode;

        // Copy the created pdf to the requested location
        if ( boost::filesystem::exists( _pdffile))
        {
            if ( !boost::filesystem::remove( _pdffile, errCode))
                _err = errCode.message();
        }   // end if

        if ( !errCode)
        {
            boost::filesystem::copy( pdfout, _pdffile);
            if ( !boost::filesystem::remove( pdfout, errCode))
                _err = errCode.message();
        }   // end if
        else
            _pdffile = pdfout.string();
    }   // end if
    else
        _pdffile = "";

    _pupdater.processUpdate(1.0f);
    return _err.empty();
}   // end doAction

