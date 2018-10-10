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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_INTERFACE_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_INTERFACE_H

#include <PluginInterface.h>    // QTools
#include <FaceModel.h>          // FaceTools

namespace FaceTools {
namespace FileIO {

// Must be pure virtual to be a plugin interface
class FaceTools_EXPORT FaceModelFileHandlerInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual ~FaceModelFileHandlerInterface(){}

    virtual QString getFileDescription() const = 0; // Human readable file type description.
    virtual const QStringSet& getFileExtensions() const = 0; // Get file extensions capable of being handled.

    // Override either or both to declare if can import or export.
    virtual bool canRead() const = 0;
    virtual bool canWrite() const = 0;

    // Returns error string on failed read/write.
    virtual QString error() const = 0;

    virtual double version() const = 0; // File version read in

    // Read and/or write according to whether canImport() and canExport() are
    // overridden. If read() returns a NULL model, error() should be used to
    // find out why no model could be created. Similarly, if write() returns
    // false, use error() to get the nature of the issue.
    virtual FaceModel* read( const QString& filepath) = 0;               // Must override if canRead overridden to true
    virtual bool write( const FaceModel*, const QString& filepath) = 0;  // Must override if canWrite overridden to true
};  // end class

}   // end namespace
}   // end namespace

#endif
