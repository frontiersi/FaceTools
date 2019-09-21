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

#include <FileIO/FaceModelAssImpFileHandler.h>
#include <cassert>
using FaceTools::FileIO::FaceModelAssImpFileHandler;
using FaceTools::FM;

// private
FaceModelAssImpFileHandler::FaceModelAssImpFileHandler( RModelIO::AssetImporter* importer, const QString& qext)
    : _assimp(importer)
{
    const std::string ext = qext.toLower().toStdString();
    assert( _assimp->getAvailable().count(ext) > 0);
    _fdesc = _assimp->getAvailable().at(ext).c_str();
    _exts.insert( qext.toLower());
}   // end ctor


// public
FM* FaceModelAssImpFileHandler::read( const QString& qfname)
{
    _err = "";
    FM* fm = nullptr;
    const std::string fname = qfname.toStdString();
    RFeatures::ObjModel::Ptr model = _assimp->load(fname);
    if ( model)
        fm = new FM(model);
    else
        _err = _assimp->err().c_str();
    return fm;
}   // end read
