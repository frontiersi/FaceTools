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

#include <ReportManager.h>
#include <PDFGenerator.h>
#include <U3DExporter.h>
#include <QFile>
#include <QDir>
#include <iostream>
#include <cassert>
using FaceTools::Report::ReportManager;
using FaceTools::Report::Report;
using RModelIO::PDFGenerator;
using RModelIO::U3DExporter;

// Static definitions
QTemporaryDir ReportManager::_tmpdir;
QString ReportManager::_hname;
QString ReportManager::_logopath;
QString ReportManager::_logofile;
QString ReportManager::_inkscape;
QStringList ReportManager::_names;
std::unordered_map<QString, Report::Ptr> ReportManager::_reports;


bool ReportManager::init( const QString& pdflatex, const QString& idtfConverter, const QString& inkscape)
{
    PDFGenerator::pdflatex = pdflatex.toStdString();
    U3DExporter::IDTFConverter = idtfConverter.toStdString();
    std::cerr << "IDTFConverter : " << U3DExporter::IDTFConverter << " ("
              << (U3DExporter::isAvailable()  ? "" : "not ") << "available)" << std::endl;
    std::cerr << "pdflatex      : " << PDFGenerator::pdflatex << " ("
              << (PDFGenerator::isAvailable() ? "" : "not ") << "available)" << std::endl;

    _inkscape = inkscape;
    //_tmpdir.setAutoRemove(false);   // Uncomment for debug

    // (Re)Create the views file in the temporary directory
    QFile viewsfile( _tmpdir.filePath("views.vws"));
    if ( viewsfile.exists())
        viewsfile.remove();

    viewsfile.open( QIODevice::ReadWrite | QIODevice::Text);
    if ( !viewsfile.isOpen())
    {
        const std::string werr = "[WARNING] FaceTools::Report::ReportManager::init: ";
        std::cerr << werr << "Unable to open '" << viewsfile.fileName().toStdString() << "' for writing!" << std::endl;
        return false;
    }   // end if

    QTextStream ots( &viewsfile);
    ots << "VIEW=Front" << endl
        << "\tROO=300" << endl
        << "\tC2C=0 -1 0" << endl
        << "\tLIGHTS=AmbientLight" << endl
        << "END" << endl
        << "VIEW=Right" << endl
        << "\tROO=300" << endl
        << "\tC2C=-1 0 0" << endl
        << "\tLIGHTS=AmbientLight" << endl
        << "END" << endl
        << "VIEW=Left" << endl
        << "\tROO=300" << endl
        << "\tC2C=1 0 0" << endl
        << "\tLIGHTS=AmbientLight" << endl
        << "END" << endl;
    viewsfile.close();

    return isAvailable();
}   // end init


bool ReportManager::isAvailable()
{
    return U3DExporter::isAvailable() && PDFGenerator::isAvailable();
}   // end isAvailable


int ReportManager::load( const QString& sdir)
{
    // Copy the logo into the temporary directory for latex to include
    QFile logo( _logopath);
    _logofile = _tmpdir.filePath("logo.pdf");
    logo.copy( _logofile);

    _names.clear();
    _reports.clear();

    QDir rdir( sdir);
    if ( !rdir.exists() || !rdir.isReadable())
    {
        static const std::string werr = "[WARNING] FaceTools::Report::ReportManager::load: ";
        std::cerr << werr << "Unable to open directory: " << sdir.toStdString() << std::endl;
        return -1;
    }   // end if

    const QStringList fnames = rdir.entryList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    int nrecs = 0;
    for ( const QString& fname : fnames)
    {
        if ( !add( rdir.absoluteFilePath( rdir.absoluteFilePath(fname))).isEmpty())
            nrecs++;
    }   // end for

    return nrecs;
}   // end load


QString ReportManager::add( const QString& file)
{
    static const std::string werr = "[WARNING] FaceTools::Report::ReportManager::add: ";
    Report::Ptr rep = Report::load( file, _tmpdir);
    if ( !rep)
        return "";

    rep->setLogo(_logofile);
    rep->setHeaderName(_hname);
    rep->setInkscape(_inkscape);

    // Check to see if the name exists already because plugins might be overriding
    if ( _reports.count(rep->name()) == 0)
    {
        _names.append(rep->name());
        _names.sort();
    }   // end if
#ifndef NDEBUG
    else
        std::cerr << werr << "Overwriting report " << rep->name().toStdString() << std::endl;
#endif

    _reports[rep->name()] = rep;    // Possibly overwrites existing!
    return rep->name();
}   // end add
