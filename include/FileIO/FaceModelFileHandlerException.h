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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_EXCEPTION_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_HANDLER_EXCEPTION_H

#ifdef _WIN32
#pragma warning( disable : 4251)    // Disable warning about non dll-interface class std::runtime_error
#pragma warning( disable : 4275)    // Disable warning about non dll-interface class std::string
#endif

#include <FaceTools_Export.h>
#include <string>
#include <stdexcept>

namespace FaceTools {
namespace FileIO {

class FaceTools_EXPORT FaceModelFileHandlerException : public std::runtime_error
{
public:
    FaceModelFileHandlerException( const std::string& ext, const std::string& what_arg);

    const std::string& ext() const { return _ext;}

private:
    const std::string _ext;
};  // end class

}   // end namespace
}   // end namespace

#endif
