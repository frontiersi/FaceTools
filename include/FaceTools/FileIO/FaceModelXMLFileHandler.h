/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_XML_FILE_HANDLER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_XML_FILE_HANDLER_H

#include "FaceModelFileHandler.h"
#include <QTemporaryDir>

namespace FaceTools { namespace FileIO {

static const std::string XML_VERSION = "5.0";
static const std::string XML_FILE_EXTENSION = "3df";
static const std::string XML_FILE_DESCRIPTION = "3D Face Image and Metadata";

class FaceTools_EXPORT FaceModelXMLFileHandler : public FaceModelFileHandler
{
public:
    FaceModelXMLFileHandler() { _exts.insert( XML_FILE_EXTENSION.c_str());}
    QString getFileDescription() const override { return XML_FILE_DESCRIPTION.c_str();}
    const QStringSet& getFileExtensions() const override { return _exts;}

    bool canRead() const override { return true;}
    bool canWrite() const override { return true;}
    bool canWriteTextures() const override { return true;}

    QString error() const override { return _err;}
    double version() const override { return _fversion;}

    FM* read( const QString& filepath) override;
    bool write( const FM*, const QString& filepath) override;

private:
    QStringSet _exts;
    QString _err;
    double _fversion;   // File version read in
};  // end class


// Read meta data from a 3DF file into the given property tree.
// Returns a non-empty string on error which contains the nature of the error.
std::string readMeta( const std::string &filename, QTemporaryDir &extractDir, PTree &tree);

// Import metadata from a property tree for the given model, setting file
// version and the object filename and returning true iff successful.
bool importMetaData( FM&, const PTree&, double &fversion, std::string& objfilename);

// Load the mesh data into the given FaceModel. Returns an empty string on success.
std::string loadData( FM&, const QTemporaryDir&, const std::string &objfilename);

// Export metadata about the given model into a property tree ready for writing.
// Note that because the data are being written out into a property tree, different
// export formats are available (not just XML).
void exportMetaData( const FM*, const std::string& objfilename, bool withExtras, PTree&);

}}   // end namespaces

#endif
