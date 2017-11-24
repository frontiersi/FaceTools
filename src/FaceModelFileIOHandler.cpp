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

#include <FaceModelFileIOHandler.h>
#include <MiscFunctions.h>
#include <FaceTools.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <map>
using FaceTools::FaceModelFileIOHandler;
using FaceTools::FileIOInterface;


namespace
{

QString createExtsFilter( std::vector<std::string>& exts)
{
    QString filter( "(");
    std::sort( exts.begin(), exts.end());   // Sort this filter's extensions alphanumerically
    foreach ( const std::string& ext, exts)
        filter.append( std::string("*." + ext + " ").c_str());
    filter.remove(filter.size()-1, 1);  // Remove end space
    filter += ")";
    return filter;
}   // end createExtsFilter


QStringList createFilters( const boost::unordered_map<std::string, boost::unordered_set<std::string> >& dsmap)
{
    QStringList allfilters;

    // Get and sort the descriptions alphanumerically
    typedef std::pair<std::string, boost::unordered_set<std::string> > FPair;
    std::vector<std::string> descs;
    foreach ( const FPair& fp, dsmap)
        descs.push_back( fp.first);
    std::sort( descs.begin(), descs.end());

    foreach ( const std::string& desc, descs)
    {
        std::vector<std::string> exts( dsmap.at(desc).begin(), dsmap.at(desc).end());
        QString filter = QString( std::string( desc + " ").c_str()) + createExtsFilter( exts);
        allfilters << filter;
    }   // end foreach
    return allfilters;
}   // end createFilters

}   // end namespace


// public
FaceModelFileIOHandler::FaceModelFileIOHandler()
{
}   // end ctor


// public
QString FaceModelFileIOHandler::createAllImportFilter() const
{
    typedef std::pair<std::string, std::string> FPair;
    std::vector<std::string> exts;
    foreach ( const FPair& ep, _importExtDescMap)
        exts.push_back(ep.first);
    return "All supported " + createExtsFilter(exts);
}   // end createAllImportFilter


// public
QString FaceModelFileIOHandler::createAllExportFilter() const
{
    typedef std::pair<std::string, std::string> FPair;
    std::vector<std::string> exts;
    foreach ( const FPair& ep, _exportExtDescMap)
        exts.push_back(ep.first);
    return "All supported " + createExtsFilter(exts);
}   // end createAllExportFilter


// public
QString FaceModelFileIOHandler::createImportFilters( bool prependAll) const
{
    QStringList filters = createFilters( _importDescExtMap);
    if ( prependAll)
        filters.prepend( createAllImportFilter());
    return filters.join(";;");
}   // end createImportFilters


// public
QString FaceModelFileIOHandler::createExportFilters( bool prependAll) const
{
    QStringList filters = createFilters( _exportDescExtMap);
    if ( prependAll)
        filters.prepend( createAllExportFilter());
    return filters.join(";;");
}   // end createExportFilters


// public
void FaceModelFileIOHandler::printIOFormats( std::ostream& os)
{
    typedef std::pair<std::string, FileIOInterface*> FPair;
    foreach ( const FPair& fp, _fileInterfaces)
    {
        const FileIOInterface* fileio = fp.second;
        const std::string& desc = fileio->getFileDescription();
        if ( fileio->canImport())
            os << "LOAD ";
        else
            os << "     ";
        if ( fileio->canExport())
            os << "SAVE ";
        else
            os << "     ";
        os << desc << " (*." << fp.first << ")" << std::endl;
    }   // end foreach
}   // end printIOFormats


// public
QString* FaceModelFileIOHandler::getFilter( const std::string& ext)
{
    const std::string sxt = boost::algorithm::to_lower_copy(ext);
    if ( _fileInterfaces.count(sxt) == 0)
        return NULL;
    const std::string desc = _fileInterfaces.at(sxt)->getFileDescription();
    _lastFilter = std::string( desc + " (*." + sxt + ")").c_str();
    return &_lastFilter;
}   // end getFilter


// public
FileIOInterface* FaceModelFileIOHandler::getLoadInterface( const std::string& fname) const
{
    FileIOInterface* fileio = NULL;
    const std::string fext = FaceTools::getExtension(fname);
    if ( _importExtDescMap.count(fext) > 0 && _fileInterfaces.count(fext) > 0)
        fileio = _fileInterfaces.at(fext);
    return fileio;
}   // end getLoadInterface


// public
FileIOInterface* FaceModelFileIOHandler::getSaveInterface( const std::string& fname) const
{
    FileIOInterface* fileio = NULL;
    const std::string fext = FaceTools::getExtension(fname);
    if ( _exportExtDescMap.count(fext) > 0 && _fileInterfaces.count(fext) > 0)
        fileio = _fileInterfaces.at(fext);
    return fileio;
}   // end getSaveInterface


// public
void FaceModelFileIOHandler::addFileFormat( FileIOInterface* fileio)
{
    assert(fileio);
    std::vector<std::string> exts;
    fileio->getFileExtensions( exts);
    foreach ( const std::string& ext, exts)
    {
        _fileInterfaces[ext] = fileio;

        if ( fileio->canImport())
        {
            _importExtDescMap[ext] = fileio->getFileDescription();
            _importDescExtMap[fileio->getFileDescription()].insert(ext);
        }   // end if

        if ( fileio->canExport())
        {
            _exportExtDescMap[ext] = fileio->getFileDescription();
            _exportDescExtMap[fileio->getFileDescription()].insert(ext);
        }   // end if

        if ( _primaryExt.empty())
            _primaryExt = ext;
    }   // end foreach
}   // end addFileFormat
