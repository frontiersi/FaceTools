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

#include <FaceModelFileHandlerMap.h>
#include <MiscFunctions.h>
#include <FaceTools.h>
#include <FileIO.h> // rlib
#include <algorithm>
#include <QStringList>
using FaceTools::FileIO::FaceModelFileHandlerMap;
using FaceTools::FileIO::FaceModelFileHandler;


namespace {

// e.g. {"*.cc", "*.cpp", "*.cxx"}
QStringList createSimpleFilter( const std::unordered_map<QString, QString>& edmap)
{
    QStringList exts;
    std::for_each( std::begin(edmap), std::end(edmap), [&](const std::pair<QString, QString>& p){ exts << "*." + p.first;});
    exts.sort();    // Sort alphanumerically
    return exts;
}   // end createSimpleFilter


// e.g. {"cc", "cpp", "cxx"}
QStringList createRawFilterList( const std::unordered_map<QString, QString>& edmap)
{
    QStringList exts;
    std::for_each( std::begin(edmap), std::end(edmap), [&](const std::pair<QString, QString>& p){ exts << p.first;});
    exts.sort();    // Sort alphanumerically
    return exts;
}   // end createRawFilterList


QString createParenthesisedFilter( QStringList& exts)
{
    QString filter( "(");
    filter.append( exts.join( ' '));
    return filter.append( ")");
}   // end createParenthesisedFilter


// dsmap maps descriptions to file extensions
QStringList createFilters( const std::unordered_map<QString, QStringSet>& dsmap)
{
    QStringList descs; // Get and sort the descriptions alphanumerically
    std::for_each( std::begin(dsmap), std::end(dsmap), [&](const std::pair<QString, QStringSet>& fp){ descs.push_back( fp.first);});
    descs.sort();

    QStringList allfilters;
    for ( const QString& desc : descs)
    {
        const QStringSet& eset = dsmap.at(desc);
        QStringList exts;
        std::for_each( std::begin(eset), std::end(eset), [&](const QString& e){ exts << "*." + e;});
        exts.sort();
        allfilters << (desc + " " + createParenthesisedFilter( exts));
    }   // end foreach
    return allfilters;
}   // end createFilters

}   // end namespace


// public
QStringList FaceModelFileHandlerMap::createSimpleImportFilters() const { return createSimpleFilter(_importExtDescMap);}
QStringList FaceModelFileHandlerMap::createSimpleExportFilters() const { return createSimpleFilter(_exportExtDescMap);}

QStringList FaceModelFileHandlerMap::createRawImportFilters() const { return createRawFilterList(_importExtDescMap);}
QStringList FaceModelFileHandlerMap::createRawExportFilters() const { return createRawFilterList(_exportExtDescMap);}

// public
QString FaceModelFileHandlerMap::createAllImportFilter() const
{
    QStringList ifilters = createSimpleImportFilters();
    return createParenthesisedFilter( ifilters).prepend("All supported ");
}   // end createAllImportFilter


// public
QString FaceModelFileHandlerMap::createAllExportFilter() const
{
    QStringList efilters = createSimpleExportFilters();
    return createParenthesisedFilter( efilters).prepend("All supported ");
}   // end createAllExportFilter


// public
QString FaceModelFileHandlerMap::createImportFilters( bool prependAll) const
{
    QStringList filters = createFilters( _importDescExtMap);
    if ( prependAll)
        filters.prepend( createAllImportFilter());
    return filters.join(";;");
}   // end createImportFilters


// public
QString FaceModelFileHandlerMap::createExportFilters( bool prependAll) const
{
    QStringList filters = createFilters( _exportDescExtMap);
    if ( prependAll)
        filters.prepend( createAllExportFilter());
    return filters.join(";;");
}   // end createExportFilters


// public
QString FaceModelFileHandlerMap::getFilter( const QString& ext) const
{
    QString sxt = ext.toLower();
    if ( _fileInterfaces.count(sxt) == 0)
        return QString();
    QString desc = _fileInterfaces.at(sxt)->getFileDescription();
    return desc + " (*." + sxt + ")";
}   // end getFilter


// public
FaceModelFileHandler* FaceModelFileHandlerMap::readInterface( const std::string& fname) const
{
    FaceModelFileHandler* fileio = nullptr;
    QString fext = rlib::getExtension(fname).c_str();
    if ( _importExtDescMap.count(fext) == 1 && _fileInterfaces.count(fext) == 1)
        fileio = _fileInterfaces.at(fext);
    return fileio;
}   // end readInterface


// public
FaceModelFileHandler* FaceModelFileHandlerMap::writeInterface( const std::string& fname) const
{
    FaceModelFileHandler* fileio = nullptr;
    QString fext = rlib::getExtension(fname).c_str();
    if ( _exportExtDescMap.count(fext) == 1 && _fileInterfaces.count(fext) == 1)
        fileio = _fileInterfaces.at(fext);
    return fileio;
}   // end writeInterface


// public
void FaceModelFileHandlerMap::add( FaceModelFileHandler* fileio)
{
    const QStringSet& exts = fileio->getFileExtensions();
    for ( const QString& ext : exts)
    {
        _fileInterfaces[ext] = fileio;

        if ( fileio->canRead())
        {
            _importExtDescMap[ext] = fileio->getFileDescription();
            _importDescExtMap[fileio->getFileDescription()].insert(ext);
        }   // end if

        if ( fileio->canWrite())
        {
            _exportExtDescMap[ext] = fileio->getFileDescription();
            _exportDescExtMap[fileio->getFileDescription()].insert(ext);
        }   // end if

        if ( _primaryExt.isEmpty())
            _primaryExt = ext;
    }   // end for
}   // end add


// public
std::ostream& FaceTools::FileIO::operator<<( std::ostream& os, const FaceModelFileHandlerMap& fmap)
{
    for ( const auto& fp : fmap._fileInterfaces)
    {
        const FaceModelFileHandler* fileio = fp.second;
        QString desc = fileio->getFileDescription();
        if ( fileio->canRead())
            os << "LOAD ";
        else
            os << "     ";
        if ( fileio->canWrite())
            os << "SAVE ";
        else
            os << "     ";
        os << desc.toStdString() << " (*." << fp.first.toStdString() << ")" << std::endl;
    }   // end for
    return os;
}   // end operator<<
