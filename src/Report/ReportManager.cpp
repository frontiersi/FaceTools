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

// Static definitions
QStringList ReportManager::_names;
std::unordered_map<QString, Report::Ptr> ReportManager::_reports;


bool ReportManager::isAvailable()
{
    return r3dio::U3DExporter::isAvailable() && r3dio::PDFGenerator::isAvailable();
}   // end isAvailable


bool ReportManager::init( const QString& pdflatex, const QString& idtfConverter)
{
    r3dio::U3DExporter::IDTFConverter = idtfConverter.toStdString();
    r3dio::PDFGenerator::pdflatex = pdflatex.toStdString();
    if ( !isAvailable())
        std::cerr << "'pdflatex' not set/found; report generation is disabled." << std::endl;
    return isAvailable();
}   // end init


int ReportManager::load( const QString& sdir)
{
    static const std::string err = "[ERROR] FaceTools::Report::ReportManager::load: ";
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
    Report::Ptr rep = Report::load( file);
    if ( !rep)
        return "";
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
