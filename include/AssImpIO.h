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

#ifndef FACE_TOOLS_ASS_IMP_IO_H
#define FACE_TOOLS_ASS_IMP_IO_H

#include "FileIOInterface.h"
#include <AssetImporter.h>   // RModelIO

namespace FaceTools {

class FaceTools_EXPORT AssImpIO : public FileIOInterface
{ Q_OBJECT
public:
    explicit AssImpIO( const std::string& ext);
    virtual ~AssImpIO();

    // QTools::PluginInterface
    virtual AssImpIO* getInterface( const QString&) { return this;}
    virtual QString getDisplayName() const { return _ext.c_str();}

    virtual std::string getFileDescription() const;
    virtual size_t getFileExtensions( std::vector<std::string>& exts) const;

    virtual bool canImport() const { return _importer != NULL;}
    virtual bool canExport() const { return false;}

protected:
    virtual FaceModel* load( const std::string& fname);

private:
    const std::string _ext;
    RModelIO::AssetImporter* _importer;
    const boost::unordered_map<std::string, std::string>* available() const;
};  // end class

}   // end namespace

#endif


