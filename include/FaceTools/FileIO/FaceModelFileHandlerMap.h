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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_MAP_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_MAP_H

/**
 * Utility class for FaceModelManager.
 * Holds references to FaceModelFileHandler instances and provides functions for looking
 * up the right file handler for a given extension/filename plus creating import and
 * export filters for use in Qt FileDialogs.
 */

#include <FaceTools/FaceTypes.h>
#include "FaceModelFileHandler.h"

namespace FaceTools { namespace FileIO {

// Prints the IO formats to given stream.
FaceTools_EXPORT std::ostream& operator<<( std::ostream&, const FaceModelFileHandlerMap&);

class FaceTools_EXPORT FaceModelFileHandlerMap
{
public:
    void add( FaceModelFileHandler*);  // The first added will be the preferred file format.
    const QString& preferredExt() const { return _primaryExt;}   // First added

    // Get the read/write interface appropriate for given filename (its extension is checked for type).
    FaceModelFileHandler* readInterface( const QString&) const;
    FaceModelFileHandler* writeInterface( const QString&) const;

    QString createAllImportFilter() const;  // Consider using "Any file (*.*)" instead if long.
    QString createAllExportFilter() const;  // Consider using "Any file (*.*)" instead if long.

    QStringList createImportFilters( bool prependAll=false) const;
    QStringList createExportFilters( bool prependAll=false) const;

    // Create import/export filters from a given subset of extensions
    QStringList createImportFilters( const QStringSet &exts, bool prependAll=false) const;
    QStringList createExportFilters( const QStringSet &exts, bool prependAll=false) const;

    QStringSet importExtensions() const;
    QStringSet exportExtensions() const;

    QString filterForExtension( const QString& ext) const;      // Gets file dialog filter for extension (empty if invalid ext)
    QString extensionForFilter( const QString& filter) const;

    QStringList createSimpleImportFilters() const;    // e.g. {"*.cpp", "*.cxx", "*.cc"}
    QStringList createSimpleExportFilters() const;    // Same but for export types

    QStringList createRawImportFilters() const;     // e.g. {"cpp", "cxx", "cc"}
    QStringList createRawExportFilters() const;     // Same but for export types

private:
    std::unordered_map<QString, FaceModelFileHandler*> _fileInterfaces; // Keyed by extensions
    std::unordered_map<QString, QString> _importExtDescMap;    // Maps extensions to descriptions
    std::unordered_map<QString, QString> _exportExtDescMap;    // Maps extensions to descriptions
    std::unordered_map<QString, QStringSet> _importDescExtMap; // Maps descriptions to extensions
    std::unordered_map<QString, QStringSet> _exportDescExtMap; // Maps descriptions to extensions
    QString _primaryExt;

    friend std::ostream& FaceTools::FileIO::operator<<( std::ostream&, const FaceModelFileHandlerMap&);
    friend class FaceModelManager;
    FaceModelFileHandlerMap(){}
    FaceModelFileHandlerMap( const FaceModelFileHandlerMap&) = delete;
    void operator=( const FaceModelFileHandlerMap&) = delete;
};  // end class

}}   // end namespace

#endif
