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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_OBJ_FILE_HANDLER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_OBJ_FILE_HANDLER_H

#include <AssetImporter.h>  // RModelIO
#include <OBJExporter.h>    // RModelIO
#include "FaceModelFileHandler.h"

namespace FaceTools {
namespace FileIO {

class FaceModelOBJFileHandler : public FaceModelFileHandler
{ Q_OBJECT
public:
    FaceModelOBJFileHandler();
    QString getFileDescription() const override;
    const std::unordered_set<QString>& getFileExtensions() const override { return _exts;}

    bool canRead() const override { return true;}
    bool canWrite() const override { return true;}

    QString error() const override { return _err;}

    FaceModel* read( const QString& filepath) override;
    bool write( const FaceModel*, const QString&) override;

private:
    RModelIO::AssetImporter _importer;
    RModelIO::OBJExporter _exporter;
    std::unordered_set<QString> _exts;
    QString _err;
};  // end class

}   // end namespace
}   // end namespace

#endif
