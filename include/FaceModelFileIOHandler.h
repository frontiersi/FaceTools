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

#ifndef FACE_TOOLS_FACE_MODEL_FILE_IO_HANDLER_H
#define FACE_TOOLS_FACE_MODEL_FILE_IO_HANDLER_H

#include <QObject>
#include <QStringList>
#include "FileIOInterface.h"
#include <boost/unordered_set.hpp>

namespace FaceTools
{

class FaceTools_EXPORT FaceModelFileIOHandler
{
public:
    FaceModelFileIOHandler();

    void addFileFormat( FileIOInterface*);  // The first added will be the preferred file format.

    // Get the load/save interface appropriate for the given file type (extension checked for type).
    FileIOInterface* getLoadInterface( const std::string& fname) const;
    FileIOInterface* getSaveInterface( const std::string& fname) const;

    QString createAllImportFilter() const;  // May be too long - consider using "Any file (*.*)" instead.
    QString createAllExportFilter() const;  // As above.
    QString createImportFilters( bool prependAllSupported=false) const;
    QString createExportFilters( bool prependAllSupported=false) const;

    QString* getFilter( const std::string& ext);  // Gets file dialog filter for extension (NULL if invalid ext)
    const std::string& getPreferredExt() const { return _primaryExt;}

    void printIOFormats( std::ostream&);    // Debug

private:
    boost::unordered_map<std::string, FileIOInterface*> _fileInterfaces;
    boost::unordered_map<std::string, std::string> _importExtDescMap; // Maps extensions to descriptions
    boost::unordered_map<std::string, std::string> _exportExtDescMap; // Maps extensions to descriptions
    boost::unordered_map<std::string, boost::unordered_set<std::string> > _importDescExtMap; // Maps descriptions to extensions
    boost::unordered_map<std::string, boost::unordered_set<std::string> > _exportDescExtMap; // Maps descriptions to extensions
    std::string _primaryExt;
    QString _lastFilter;

    FaceModelFileIOHandler( const FaceModelFileIOHandler&);
    FaceModelFileIOHandler& operator=( const FaceModelFileIOHandler&);
};  // end class

}   // end namespace

#endif

