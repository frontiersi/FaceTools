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

#include <Report/ReportManager.h>
#include <r3dio/PDFGenerator.h>
#include <r3dio/U3DExporter.h>
#include <QFile>
#include <QDir>
#include <iostream>
#include <cassert>
using FaceTools::Report::ReportManager;
using FaceTools::Report::Report;
using r3dio::PDFGenerator;
using r3dio::U3DExporter;

// Static definitions
QTemporaryDir ReportManager::_tmpdir;
QString ReportManager::_hname;
QString ReportManager::_version;
QString ReportManager::_logopath;
QString ReportManager::_logofile;
QString ReportManager::_inkscape;
QStringList ReportManager::_names;
std::unordered_map<QString, Report::Ptr> ReportManager::_reports;



bool ReportManager::init( const QString& pdflatex, const QString& idtfConverter, const QString& inkscape)
{
    U3DExporter::IDTFConverter = idtfConverter.toStdString();
    if ( !U3DExporter::isAvailable())
        std::cerr << "WARNING: IDTFConverter " << U3DExporter::IDTFConverter << " unavailable!" << std::endl;

    PDFGenerator::pdflatex = pdflatex.toStdString();
    if ( !PDFGenerator::isAvailable())
        std::cerr << "WARNING: pdflatex " << PDFGenerator::pdflatex << " unavailable!" << std::endl;

    _inkscape = inkscape;
    //_tmpdir.setAutoRemove(false);   // Uncomment for debug

    // Write the Javascript file that hides the view axes
    QFile hideAxesFile( _tmpdir.filePath("hideAxes.js"));
    if ( hideAxesFile.exists())
        hideAxesFile.remove();
    hideAxesFile.open( QIODevice::ReadWrite | QIODevice::Text);
    if ( !hideAxesFile.isOpen())
    {
        const std::string werr = "[WARNING] FaceTools::Report::ReportManager::init: ";
        std::cerr << werr << "Unable to open 'hideAxes.js' for writing!" << std::endl;
        return false;
    }   // end if
    QTextStream os( &hideAxesFile);
    os << "scene.showOrientationAxes = false;" << Qt::endl;
    hideAxesFile.close();

    return isAvailable();
}   // end init


bool ReportManager::writeViewsFile( float d, const QString &fname)
{
    // (Re)Create the views file in the temporary directory
    QFile viewsfile( _tmpdir.filePath(fname));
    if ( viewsfile.exists())
        viewsfile.remove();

    viewsfile.open( QIODevice::ReadWrite | QIODevice::Text);
    if ( !viewsfile.isOpen())
    {
        const std::string werr = "[WARNING] FaceTools::Report::ReportManager::writeViewsFile: ";
        std::cerr << werr << "Unable to open '" << viewsfile.fileName().toStdString() << "' for writing!" << std::endl;
        return false;
    }   // end if

    QTextStream ots( &viewsfile);
    ots << "VIEW=Front" << Qt::endl
        << "  C2C=0 -1 0" << Qt::endl
        << "  ROO=" << d << Qt::endl
        << "  LIGHTS=AmbientLight" << Qt::endl
        << "END" << Qt::endl
        << "VIEW=Right" << Qt::endl
        << "  C2C=-1 0 0" << Qt::endl
        << "  ROO=" << d << Qt::endl
        << "  LIGHTS=AmbientLight" << Qt::endl
        << "END" << Qt::endl
        << "VIEW=Left" << Qt::endl
        << "  C2C=1 0 0" << Qt::endl
        << "  ROO=" << d << Qt::endl
        << "  LIGHTS=AmbientLight" << Qt::endl
        << "END" << Qt::endl;
    viewsfile.close();
    return true;
}   // end writeViewsFile


bool ReportManager::isAvailable()
{
    return U3DExporter::isAvailable() && PDFGenerator::isAvailable();
}   // end isAvailable


int ReportManager::load( const QString& sdir)
{
    static const std::string err = "[ERROR] FaceTools::Report::ReportManager::load: ";

    // Copy the logo into the report generation directory
    _logofile = _tmpdir.filePath("logo.pdf");
    if ( !QFile( _logopath).copy( _logofile))
    {
        std::cerr << err << "Unable to copy logo to report generation directory!" << std::endl;
        return -1;
    }   // end if

    _names.clear();
    _reports.clear();

    QDir rdir( sdir);
    if ( !rdir.exists() || !rdir.isReadable())
    {
        std::cerr << err << "Unable to open directory: " << sdir.toStdString() << std::endl;
        return -1;
    }   // end if

    const QStringList fnames = rdir.entryList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    int nrecs = 0;
    for ( const QString& fname : fnames)
        if ( !add( rdir.absoluteFilePath( rdir.absoluteFilePath(fname))).isEmpty())
            nrecs++;

    return nrecs;
}   // end load


QString ReportManager::add( const QString& file)
{
    static const std::string werr = "[WARNING] FaceTools::Report::ReportManager::add: ";
    Report::Ptr rep = Report::load( file, _tmpdir);
    if ( !rep)
        return "";

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
        std::cerr << werr << " Overwriting report " << rep->name().toStdString() << std::endl;
#endif

    _reports[rep->name()] = rep;    // Possibly overwrites existing!
    return rep->name();
}   // end add
