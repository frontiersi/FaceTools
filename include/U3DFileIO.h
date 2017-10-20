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

#ifndef FACE_TOOLS_U3D_FILE_IO_H
#define FACE_TOOLS_U3D_FILE_IO_H

#include "FileIOInterface.h" // FaceTools
#include <U3DExporter.h>     // RModelIO

namespace FaceTools {

class FaceTools_EXPORT U3DFileIO : public FileIOInterface
{ Q_OBJECT
public:
    U3DFileIO();
    virtual ~U3DFileIO();

    // QTools::PluginInterface
    virtual U3DFileIO* getInterface( const QString&) { return this;}
    virtual QString getDisplayName() const;

    virtual std::string getFileDescription() const { return getDisplayName().toStdString();}
    virtual size_t getFileExtensions( std::vector<std::string>& exts) const;

    virtual bool canImport() const { return false;}
    virtual bool canExport() const;

protected:
    virtual FaceModel* load( const std::string&) { return NULL;} // Not supported
    virtual bool save( const FaceModel*, const std::string&);

private:
    RModelIO::U3DExporter* _exporter;
};  // end class

}   // end namespace FaceApp

#endif


