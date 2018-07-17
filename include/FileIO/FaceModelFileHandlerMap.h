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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_MAP_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_MAP_H

/**
 * Utility class for FaceModelManager.
 * Holds references to FaceModelFileHandler instances and provides functions for looking
 * up the right file handler for a given extension/filename plus creating import and
 * export filters for use in Qt FileDialogs.
 */

#include <Hashing.h>
#include <iostream>
#include <unordered_map>
#include "FaceModelFileHandler.h"

namespace FaceTools {
namespace FileIO {

class FaceModelManager;
class FaceModelFileHandlerMap;

// Prints the IO formats to given stream.
FaceTools_EXPORT std::ostream& operator<<( std::ostream&, const FaceModelFileHandlerMap&);

class FaceTools_EXPORT FaceModelFileHandlerMap
{
public:
    void add( FaceModelFileHandler*);  // The first added will be the preferred file format.
    const QString& preferredExt() const { return _primaryExt;}   // First added

    // Get the load/save interface appropriate for the given filename (extension checked for type).
    FaceModelFileHandler* getLoadInterface( const std::string&) const;
    FaceModelFileHandler* getSaveInterface( const std::string&) const;

    QString createAllImportFilter() const;  // Consider using "Any file (*.*)" instead if long.
    QString createAllExportFilter() const;  // Consider using "Any file (*.*)" instead if long.
    QString createImportFilters( bool prependAll=false) const;
    QString createExportFilters( bool prependAll=false) const;

    QString getFilter( const QString& ext) const;  // Gets file dialog filter for extension (empty if invalid ext)

    QStringList createSimpleImportFilters() const;    // e.g. the three element list "*.cpp", "*.cxx", "*.cc" (no descriptions)
    QStringList createSimpleExportFilters() const;    // As above but for export types

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
    FaceModelFileHandlerMap( const FaceModelFileHandlerMap&);   // No copy
    void operator=( const FaceModelFileHandlerMap&);            // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
