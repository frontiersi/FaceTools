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

#include <FaceModelU3DFileHandler.h>
#include <FaceModelFileHandlerException.h>
#include <algorithm>
#include <iostream>
using FaceTools::FileIO::FaceModelU3DFileHandler;
using FaceTools::FileIO::FaceModelFileHandlerException;
using FaceTools::FaceModel;


FaceModelU3DFileHandler::FaceModelU3DFileHandler( bool delFilesWhenDone)
    : _exporter( delFilesWhenDone)
{
    if ( !canWrite())
        throw FaceModelFileHandlerException( "u3d", "FaceModelU3DFileHandler unsupported on construction!");
    const std::vector<std::string>& exts = _exporter.getExtensions();
    std::for_each( std::begin(exts), std::end(exts), [this]( const std::string& s){ _exts.insert(s.c_str());});
}   // end ctor


QString FaceModelU3DFileHandler::getFileDescription() const
{
    return _exporter.getDescription( (*_exts.begin()).toStdString()).c_str();
}   // end getFileDescription


bool FaceModelU3DFileHandler::write( const FaceModel* fm, const QString& fname)
{
    _err = "";
    if ( !_exporter.save( fm->info()->cmodel(), fname.toStdString()))
        _err = _exporter.err().c_str();
    return _err.isEmpty();
}   // end write
