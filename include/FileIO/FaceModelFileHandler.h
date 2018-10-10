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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_H

#include <FaceModelFileHandlerInterface.h>

namespace FaceTools {
namespace FileIO {

class FaceTools_EXPORT FaceModelFileHandler : public FaceModelFileHandlerInterface
{ Q_OBJECT
public:
    FaceModelFileHandler(){}

    // Must implement FaceModelFileHandlerInterface::getFileDescription
    // Must implement FaceModelFileHandlerInterface::getFileExtensions
    // Must implement FaceModelFileHandlerInterface::error
    double version() const override { return 0;}

    bool canRead() const override { return false;}
    bool canWrite() const override { return false;}

    // Default implementations cause application exit if not overridden and respective canRead/Write return true.
    FaceModel* read( const QString& filepath) override;               // Must override if canRead overridden to true
    bool write( const FaceModel*, const QString& filepath) override;  // Must override if canWrite overridden to true

private:
    FaceModelFileHandler( const FaceModelFileHandler&) = delete;
    void operator=( const FaceModelFileHandler&) = delete;
};  // end class

}}   // end namespaces

#endif
