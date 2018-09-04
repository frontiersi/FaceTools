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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_XML_FILE_HANDLER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_XML_FILE_HANDLER_H

#include "FaceModelFileHandler.h"

namespace FaceTools {
namespace FileIO {

static const std::string XML_VERSION = "1.32";
static const std::string XML_FILE_EXTENSION = "3df";
static const std::string XML_FILE_DESCRIPTION = "Cliniface";

class FaceTools_EXPORT FaceModelXMLFileHandler : public FaceModelFileHandler
{ Q_OBJECT
public:
    FaceModelXMLFileHandler() { _exts.insert( XML_FILE_EXTENSION.c_str());}
    QString getFileDescription() const override { return XML_FILE_DESCRIPTION.c_str();}
    const QStringSet& getFileExtensions() const override { return _exts;}

    bool canRead() const override { return true;}
    bool canWrite() const override { return true;}

    QString error() const override { return _err;}

    FaceModel* read( const QString& filepath) override;
    bool write( const FaceModel*, const QString& filepath) override;

private:
    QStringSet _exts;
    QString _err;
};  // end class

}   // end namespace
}   // end namespace

#endif
