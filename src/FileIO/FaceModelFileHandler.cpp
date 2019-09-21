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

#include <FileIO/FaceModelFileHandler.h>
#include <cassert>
using FaceTools::FileIO::FaceModelFileHandler;
using FaceTools::FM;

FM* FaceModelFileHandler::read( const QString&)
{
    if ( canRead())
    {
        std::cerr << "FaceTools::FileIO::FaceModelFileHandler::read: canRead() returns true - MUST override read()!" << std::endl;
        assert(false);
        exit(-1);
    }   // end if
    return nullptr;
}   // end read


bool FaceModelFileHandler::write( const FM*, const QString&)
{
    if ( canWrite())
    {
        std::cerr << "FaceTools::FileIO::FaceModelFileHandler::write: canWrite() returns true - MUST override write()!" << std::endl;
        assert(false);
        exit(-1);
    }   // end if
    return false;
}   // end write
