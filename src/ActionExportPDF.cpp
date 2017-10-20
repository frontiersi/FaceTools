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
#include <PDFGenerator.h>      // RModelIO
#include <U3DExporter.h>       // RModelIO
#include <QFileDialog>
#include <QMessageBox>
#include <boost/filesystem.hpp>
#include <fstream>
using FaceTools::ActionExportPDF;
using FaceTools::ModelInteractor;
using FaceTools::FaceView;
using RModelIO::PDFGenerator;
using RModelIO::U3DExporter;
using RFeatures::CameraParams;
using RFeatures::ObjModel;

// public
ActionExportPDF::ActionExportPDF( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _fview(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionExportPDF::setInteractive( ModelInteractor* interactor, bool enable)
{
    _fview = NULL;
    if ( enable)
        _fview = interactor->getView();
    checkEnable();
}   // end setInteractive


void ActionExportPDF::checkEnable()
{
    setEnabled(_fview && PDFGenerator::isAvailable() && U3DExporter::isAvailable());
}   // end checkEnable


bool writeDoc( PDFGenerator& pdfgen, const std::string& texfile, const ObjModel::Ptr model, const CameraParams& cam)
{
    PDFGenerator::LaTeXU3DInserter* fig = pdfgen.getFigureInserter( model, 150, 150, cam, "Exported Model (beta)");
    if ( !fig)
        return false;

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
        os << std::endl;
        os << "\\begin{document}" << std::endl;
        os << "\\title{Facial Analysis Report (beta)}" << std::endl;
        os << "\\author{\\href{mailto:r.l.palmer@curtin.edu.au}{Richard Palmer}}" << std::endl;
        os << "\\maketitle" << std::endl;
        os << "\\thispagestyle{empty}" << std::endl;
        os << std::endl;

        os << *fig << std::endl;

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


// protected
bool ActionExportPDF::doAction()
{
    assert( _fview);
    std::string outfile = _fview->getModel()->getSaveFilepath();
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
    //fileDialog.setOption( QFileDialog::DontUseNativeDialog);

    QStringList fileNames;
    if ( fileDialog.exec())
        fileNames = fileDialog.selectedFiles();
    if ( fileNames.empty())
        return true;

    const std::string pdffile = fileNames.first().toStdString();
    // Replace extension with ".tex"
    const std::string texfile = boost::filesystem::path(pdffile).replace_extension("tex").string();
    RModelIO::PDFGenerator pdfgen;

    std::string err;
    // Currently, the model itself is written out, not the VTK view (next version!)
    if ( !writeDoc( pdfgen, texfile, _fview->getModel()->getObjectMeta()->getObject(), _fview->getCamera()))
        err = "Failed to create .tex file!";
    else if ( !pdfgen( texfile, true))
        err = "Failed to create PDF!";

    if ( !err.empty())
    {
        QMessageBox mb( tr("PDF Export Error!"), tr(err.c_str()),
                        QMessageBox::Warning,
                        QMessageBox::Close | QMessageBox::Default,
                        QMessageBox::NoButton,
                        QMessageBox::NoButton);
        mb.exec();
    }   // end if

    return err.empty();
}   // end doAction



