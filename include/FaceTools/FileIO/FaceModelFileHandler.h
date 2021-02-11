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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_H

#include <FaceTools/FaceModel.h>

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT FaceModelFileHandler
{
public:
    FaceModelFileHandler(){}
    virtual ~FaceModelFileHandler(){}

    virtual QString getFileDescription() const = 0; // Human readable file type description.
    virtual const QStringSet& getFileExtensions() const = 0; // Get file extensions capable of being handled.
    virtual QString error() const = 0;      // Returns error string on failed read/write.

    virtual double version() const { return 0;}

    virtual bool canRead() const { return false;}
    virtual bool canWrite() const { return false;}
    virtual bool canWriteTextures() const { return false;}

    // Default implementations cause application exit if not overridden and respective canRead/Write return true.
    virtual FM* read( const QString&);              // Must override if canRead overridden to true
    virtual bool write( const FM*, const QString&); // Must override if canWrite overridden to true

protected:
    static void setImageCaptureDate( FM*, const QString&);

private:
    FaceModelFileHandler( const FaceModelFileHandler&) = delete;
    void operator=( const FaceModelFileHandler&) = delete;
};  // end class

}}   // end namespaces

#endif
