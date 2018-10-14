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
#include <FaceModelManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <PDFGenerator.h>       // RModelIO
#include <U3DExporter.h>       // RModelIO

#include <QTemporaryDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>
#include <cassert>
using FaceTools::Action::ActionExportPDF;
using FaceTools::Action::EventSet;
using FaceTools::Report::BaseReportTemplate;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FaceModel;

using RModelIO::PDFGenerator;
using RModelIO::U3DExporter;
typedef RModelIO::LaTeXU3DInserter::Ptr FigIns;


// static public
bool ActionExportPDF::init( const std::string& pdflatex, const std::string& idtfConverter)
{
    PDFGenerator::pdflatex = pdflatex;
    U3DExporter::IDTFConverter = idtfConverter;
    std::cerr << "IDTFConverter : " << U3DExporter::IDTFConverter << " ("
              << (U3DExporter::isAvailable()  ? "" : "not ") << "available)" << std::endl;
    std::cerr << "pdflatex      : " << PDFGenerator::pdflatex << " ("
              << (PDFGenerator::isAvailable() ? "" : "not ") << "available)" << std::endl;
    return isAvailable();
}   // end init


// static public
bool ActionExportPDF::isAvailable()
{
    return U3DExporter::isAvailable() && PDFGenerator::isAvailable();
}   // end isAvailable


// public
ActionExportPDF::ActionExportPDF( BaseReportTemplate* t, const QIcon& icon, const QString& email, QWidget* p, QProgressBar* pb)
    : FaceAction( t->getDisplayName(), icon), _template(t), _parent(p)
{
    // Default author info
    if ( !email.isEmpty())
        _author = "\\href{mailto:" + email + "?subject=FaceTools::Action::ActionExportPDF}{Email}";
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionExportPDF::testReady( const FV* fv) { return _template->isAvailable(fv->data());}

bool ActionExportPDF::testEnabled( const QPoint*) const { return ready1() && isAvailable();}


// Get the save filepath for the report
bool ActionExportPDF::doBeforeAction( FVS& fvs, const QPoint&)
{
    assert( fvs.size() == 1);
    const FV* fv = fvs.first();
    std::string fname = FileIO::FMM::filepath( fv->data());
    QString outfile = boost::filesystem::path(fname).filename().replace_extension( "pdf").string().c_str();

    QFileDialog fileDialog;
    fileDialog.setWindowTitle( tr("Export as PDF"));
    fileDialog.setFileMode( QFileDialog::AnyFile);
    fileDialog.setNameFilter( "Portable Document Format (*.pdf)");
    //fileDialog.setDirectory( parentDir);    // Default save directory is last save location for model
    fileDialog.setDefaultSuffix( "pdf");
    fileDialog.selectFile( outfile);
    fileDialog.setAcceptMode( QFileDialog::AcceptSave);
    //fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    QStringList fileNames;
    if ( fileDialog.exec())
        fileNames = fileDialog.selectedFiles();
    fileDialog.close();

    _pdffile = "";
    if ( !fileNames.empty())
    {
        // Set the output file and try to remove what's already there (if anything)
        _pdffile = fileNames.first().toStdString();
        boost::system::error_code errCode;
        if ( boost::filesystem::exists( _pdffile))
        {
            if ( !boost::filesystem::remove( _pdffile, errCode))
            {
                _err = errCode.message();
                _pdffile = "";
            }   // end if
        }   // end if
    }   // end if

    return !_pdffile.empty();
}   // end doBeforeAction


bool ActionExportPDF::doAction( FVS& fvs, const QPoint&)
{
    assert( fvs.size() == 1);
    const FV* fv = fvs.first();
    _err = "";

    QTemporaryDir tdir;
    //tdir.setAutoRemove(false);    // Uncomment for debug purposes
    if ( !tdir.isValid())
    {
        _err = "Unable to open temporary directory for report generation!";
        return false;
    }   // end if

    progress(0.0f);

    QFile logo( _logoFile);
    const std::string logopath = tdir.filePath( "logo.pdf").toStdString();
    if ( logo.copy( logopath.c_str()))
        std::cerr << "[INFO] FaceTools::Action::ActionExportPDF::doAction: Saved logo to ";
    else
        std::cerr << "[ERROR] FaceTools::Action::ActionExportPDF::doAction: Error saving logo to ";
    std::cerr << "'" << logopath << "'" << std::endl;

    std::cerr << "[INFO] FaceTools::Action::ActionExportPDF::doAction: Creating U3D based figure(s)..." << std::endl;
    const std::string texpath = tdir.filePath( "report.tex").toStdString();   // File location where LaTeX being written
    // Figure references need to be stored until after PDF generation since LaTeXU3DInserter destructor
    // will remove delete U3D instances from the filesystem.
    std::vector<FigIns> figs;
    if ( !writeLaTeX( fv->data(), fv->viewer()->getCamera(), tdir.path().toStdString(), texpath, logopath, figs))
        _err = "Failed to create '" + texpath + "' for LaTeX parsing!";
    else
    {
        std::cerr << "[INFO] FaceTools::Action::ActionExportPDF::doAction: Generating PDF from LaTeX..." << std::endl;
        PDFGenerator pdfgen(false);
        const bool genOk = pdfgen( texpath, true);
        if ( !genOk)
            _err = "Failed to generate PDF!";
        else
        {
            // Copy the created pdf to the requested location.
            QFile outfile( tdir.filePath( "report.pdf"));
            outfile.copy( _pdffile.c_str());
        }   // end else
    }   // endelse

    progress(1.0f);
    return _err.empty();
}   // end doAction


void ActionExportPDF::doAfterAction( EventSet& cs, const FVS&, bool v)
{
    if ( !v)
    {
        QMessageBox::warning( _parent, tr("PDF Export Error!"), tr( _err.c_str()));
        std::cerr << _err << std::endl;
    }   // end if
    else 
    {
        std::cerr << "[INFO] FaceTools::Action::ActionExportPDF::doAfterAction: Created PDF at '" << _pdffile << "'" << std::endl;
        QMessageBox::information( _parent, tr("Created PDF"), tr( (std::string("Created PDF at '") + _pdffile + "'").c_str()));
        cs.insert(REPORT_CREATED);
    }   // end if
    _pdffile = "";
}   // end doAfterAction


void ActionExportPDF::purge( const FaceModel* fm) { _template->purge(fm);}


// private
bool ActionExportPDF::writeLaTeX( const FaceModel* fm,
                                  const RFeatures::CameraParams& cam,
                                  const std::string& tmpdir,
                                  const std::string& texfile,
                                  const std::string& logopath,
                                  std::vector<FigIns>& figs)
{
    std::string ainfo = _author.toStdString();
    std::ofstream os;
    try
    {
        os.open( texfile.c_str(), std::ios::out);
        os << "\\documentclass{article}" << std::endl;
        os << "\\listfiles" << std::endl;   // Do this to see in the .log file which packages are used
        os << "\\usepackage[textwidth=20cm,textheight=24cm]{geometry}" << std::endl;
        os << "\\usepackage{media9}" << std::endl;
        os << "\\usepackage[parfill]{parskip}" << std::endl;
        os << "\\usepackage[colorlinks=true,urlcolor=red]{hyperref}" << std::endl;
        os << "\\usepackage{graphicx}" << std::endl;
        os << "\\DeclareGraphicsExtensions{.png,.jpg,.pdf,.eps}" << std::endl;

        // Header
        os << std::endl;
        os << "\\usepackage{fancyhdr}" << std::endl;
        os << "\\pagestyle{fancy}" << std::endl;
        os << "\\renewcommand{\\headrulewidth}{0pt}" << std::endl;
        os << "\\renewcommand{\\footrulewidth}{0pt}" << std::endl;
        os << "\\setlength\\headheight{80.0pt}" << std::endl;
        os << "\\addtolength{\\textheight}{-80.0pt}" << std::endl;
        os << "\\rhead{\\includegraphics[width=0.5\\textwidth]{" << logopath << "}}" << std::endl;
        os << "\\lhead{" << std::endl;
        os << "\\Large " << _template->reportTitle() << " \\\\" << std::endl;
        os << "\\large \\today \\\\" << std::endl;
        if ( !ainfo.empty())
            os << "\\normalsize " << ainfo << " \\\\" << std::endl;
        os << "}" << std::endl;

        // Document
        os << std::endl;
        os << "\\begin{document}" << std::endl;
        os << "\\pagenumbering{gobble}" << std::endl;
        os << "\\thispagestyle{fancy}" << std::endl;
        os << std::endl;

        const size_t nfigs = _template->figureCount();
        const float progVal = 0.9f/nfigs;
        for ( size_t i = 0; i < nfigs; ++i)
        {
            FigIns fig = _template->createFigure( i, tmpdir, fm, cam);
            if ( fig)
            {
                figs.push_back(fig);    // Store until after PDF generated so U3D figure not prematurely deleted from filesystem.
                os << *fig << std::endl;
            }   // end if
            else
                _err = "Report template figure count mismatch!";
            progress( progress() + progVal);
        }   // end for

        os << "\\end{document}" << std::endl;
        os.flush();
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "[EXCEPTION!] Exception writing to file " << texfile << std::endl;
        std::cerr << e.what() << std::endl;
        _err = "Unable to write report template at '" + texfile + "'!";
    }   // end catch
    os.close();
    return _err.empty();
}   // end writeLaTeX


