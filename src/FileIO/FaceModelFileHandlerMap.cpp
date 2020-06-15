/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <FileIO/FaceModelFileHandlerMap.h>
#include <MiscFunctions.h>
#include <FaceTools.h>
#include <algorithm>
#include <QFileInfo>
#include <QStringList>
#include <QRegularExpression>
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


QStringList FaceModelFileHandlerMap::createSimpleImportFilters() const { return createSimpleFilter(_importExtDescMap);}
QStringList FaceModelFileHandlerMap::createSimpleExportFilters() const { return createSimpleFilter(_exportExtDescMap);}

QStringList FaceModelFileHandlerMap::createRawImportFilters() const { return createRawFilterList(_importExtDescMap);}
QStringList FaceModelFileHandlerMap::createRawExportFilters() const { return createRawFilterList(_exportExtDescMap);}

QString FaceModelFileHandlerMap::createAllImportFilter() const
{
    QStringList ifilters = createSimpleImportFilters();
    return createParenthesisedFilter( ifilters).prepend("All supported ");
}   // end createAllImportFilter


QString FaceModelFileHandlerMap::createAllExportFilter() const
{
    QStringList efilters = createSimpleExportFilters();
    return createParenthesisedFilter( efilters).prepend("All supported ");
}   // end createAllExportFilter


QStringList FaceModelFileHandlerMap::createImportFilters( bool prependAll) const
{
    QStringList filters = createFilters( _importDescExtMap);
    if ( prependAll)
        filters.prepend( createAllImportFilter());
    return filters;
}   // end createImportFilters


QStringList FaceModelFileHandlerMap::createExportFilters( bool prependAll) const
{
    QStringList filters = createFilters( _exportDescExtMap);
    if ( prependAll)
        filters.prepend( createAllExportFilter());
    return filters;
}   // end createExportFilters


QStringList FaceModelFileHandlerMap::createImportFilters( const QStringSet &exts, bool prependAll) const
{
    std::unordered_map<QString, QStringSet> descExtMap;
    for ( const QString &ext : exts)
        if ( _importExtDescMap.count(ext) > 0)
            descExtMap[_importExtDescMap.at(ext)].insert( ext);

    QStringList filters = createFilters( descExtMap);
    if ( prependAll)
        filters.prepend( createAllImportFilter());
    return filters;
}   // end createImportFilters


QStringList FaceModelFileHandlerMap::createExportFilters( const QStringSet &exts, bool prependAll) const
{
    std::unordered_map<QString, QStringSet> descExtMap;
    for ( const QString &ext : exts)
        if ( _exportExtDescMap.count(ext) > 0)
            descExtMap[_exportExtDescMap.at(ext)].insert( ext);

    QStringList filters = createFilters( descExtMap);
    if ( prependAll)
        filters.prepend( createAllImportFilter());
    return filters;
}   // end createExportFilters


QStringSet FaceModelFileHandlerMap::importExtensions() const
{
    QStringSet ss;
    for ( const auto &p : _importExtDescMap)
        ss.insert( p.first);
    return ss;
}   // end importExtensions


QStringSet FaceModelFileHandlerMap::exportExtensions() const
{
    QStringSet ss;
    for ( const auto &p : _exportExtDescMap)
        ss.insert( p.first);
    return ss;
}   // end exportExtensions


QString FaceModelFileHandlerMap::filterForExtension( const QString& ext) const
{
    QString sxt = ext.toLower();
    if ( _fileInterfaces.count(sxt) == 0)
        return QString();
    QString desc = _fileInterfaces.at(sxt)->getFileDescription();
    return desc + " (*." + sxt + ")";
}   // end filterForExtension


QString FaceModelFileHandlerMap::extensionForFilter( const QString& filter) const
{
    const QStringList toks = filter.split(QRegularExpression(R"(\(\*\..*\))"));
    const QString desc = toks.isEmpty() ? "" : toks.first().trimmed();
    QString ext;
    if ( _exportDescExtMap.count(desc) > 0)
        ext = *_exportDescExtMap.at(desc).begin();
    else if ( _importDescExtMap.count(desc) > 0)
        ext = *_importDescExtMap.at(desc).begin();
    return ext;
}   // end extensionForFilter


FaceModelFileHandler* FaceModelFileHandlerMap::readInterface( const QString& fname) const
{
    FaceModelFileHandler* fileio = nullptr;
    const QString fext = QFileInfo( fname).suffix().toLower();
    if ( _importExtDescMap.count(fext) == 1 && _fileInterfaces.count(fext) == 1)
        fileio = _fileInterfaces.at(fext);
    return fileio;
}   // end readInterface


FaceModelFileHandler* FaceModelFileHandlerMap::writeInterface( const QString& fname) const
{
    FaceModelFileHandler* fileio = nullptr;
    const QString fext = QFileInfo( fname).suffix().toLower();
    if ( _exportExtDescMap.count(fext) == 1 && _fileInterfaces.count(fext) == 1)
        fileio = _fileInterfaces.at(fext);
    return fileio;
}   // end writeInterface


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
