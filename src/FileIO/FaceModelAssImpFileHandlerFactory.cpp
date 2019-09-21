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

#include <FileIO/FaceModelAssImpFileHandlerFactory.h>
#include <FileIO/FaceModelFileHandlerException.h>
#include <algorithm>
#include <iomanip>
#include <cassert>
using FaceTools::FileIO::FaceModelAssImpFileHandlerFactory;
using FaceTools::FileIO::FaceModelFileHandlerException;
using FaceTools::FileIO::FaceModelAssImpFileHandler;


// static definition
RModelIO::AssetImporter FaceModelAssImpFileHandlerFactory::assimp( true/*load textures*/, false/*don't fail on non triangles*/);


// public
FaceModelAssImpFileHandler* FaceModelAssImpFileHandlerFactory::make( const QString& qext)
{
    const std::string ext = qext.toLower().toStdString();
    if ( assimp.getAvailable().count(ext) == 0)
        return nullptr;

    if ( !assimp.enableFormat( ext)) // Enable using provided file extension
    {
        throw FaceModelFileHandlerException( ext,
                "[ERROR] FaceTools::FileIO::FaceModelAssImpFileHandlerFactory: Unable to enable *available* file format!");
    }   // end if

    return new FaceModelAssImpFileHandler( &assimp, qext.toLower());
}   // end make


// public static
void FaceModelAssImpFileHandlerFactory::printAvailableFormats( std::ostream& os)
{
    for ( auto p : assimp.getAvailable())
        os << std::setw(10) << p.first << " | " << p.second << std::endl;
}   // end printAvailableFormats
