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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_ASS_IMP_FILE_HANDLER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_ASS_IMP_FILE_HANDLER_H

/**
 * Use FaceModelAssImpFileHandlerFactory to create new importers
 * for FaceModel instances that wrap RModelIO::AssetImporter.
 */

#include "FaceModelFileHandler.h"
#include <AssetImporter.h>   // RModelIO

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT FaceModelAssImpFileHandler : public FaceModelFileHandler
{
public:
    QString getFileDescription() const override { return _fdesc;}
    const QStringSet& getFileExtensions() const override { return _exts;}

    bool canRead() const override { return true;}
    QString error() const override { return _err;}
    FM* read( const QString&) override;

private:
    RModelIO::AssetImporter *_assimp;
    QStringSet _exts;
    QString _fdesc;
    QString _err;

    friend class FaceModelAssImpFileHandlerFactory;
    FaceModelAssImpFileHandler( RModelIO::AssetImporter*, const QString& ext);
};  // end class

}}   // end namespace

#endif
