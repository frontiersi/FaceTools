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

#include <FaceModelAssImpFileHandlerFactory.h>
#include <FaceModelFileHandlerException.h>
#include <algorithm>
#include <cassert>
using FaceTools::FileIO::FaceModelAssImpFileHandlerFactory;
using FaceTools::FileIO::FaceModelFileHandlerException;
using FaceTools::FileIO::FaceModelAssImpFileHandler;

// public
FaceModelAssImpFileHandlerFactory::FaceModelAssImpFileHandlerFactory()
    : _importer( true/*load textures*/, true/*fail on non triangles*/)
{
}   // end ctor


// public
FaceModelAssImpFileHandlerFactory::~FaceModelAssImpFileHandlerFactory()
{
    std::for_each( std::begin(_fhandlers), std::end(_fhandlers), [this]( auto p){ delete p.second;});
}   // end dtor


// public
FaceModelAssImpFileHandler* FaceModelAssImpFileHandlerFactory::get( const QString& qext)
{
    const std::string ext = qext.toLower().toStdString();
    if ( _importer.getAvailable().count(ext) == 0)
        return NULL;

    if ( !_importer.enableFormat( ext)) // Enable using provided file extension
    {
        throw FaceModelFileHandlerException( ext, "[ERROR] FaceTools::FileIO::FaceModelAssImpFileHandlerFactory: Unable to enable *available* file format!");
    }   // end if

    if ( _fhandlers.count(ext) == 0)
        _fhandlers[ext] = new FaceModelAssImpFileHandler( &_importer, qext.toLower());
    return _fhandlers.at(ext);
}   // end get
