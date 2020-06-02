/************************************************************************
 * Copyright (C) 2018 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_U3D_FILE_HANDLER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_U3D_FILE_HANDLER_H

#include "FaceModelFileHandler.h"
#include <r3dio/U3DExporter.h>

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT FaceModelU3DFileHandler : public FaceModelFileHandler
{
public:
    explicit FaceModelU3DFileHandler( bool delFilesWhenDone=true);
    QString getFileDescription() const override;
    const QStringSet& getFileExtensions() const override { return _exts;}

    bool canWrite() const override { return _exporter.isSupported();}
    bool canWriteTextures() const override { return true;}

    QString error() const override { return _err;}
    bool write( const FM*, const QString& filepath) override;

private:
    r3dio::U3DExporter _exporter;
    QStringSet _exts;
    QString _err;
};  // end class

}}   // end namespaces

#endif
