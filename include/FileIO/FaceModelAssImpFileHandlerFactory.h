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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_ASS_IMP_FILE_HANDLER_FACTORY_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_ASS_IMP_FILE_HANDLER_FACTORY_H

#include "FaceModelAssImpFileHandler.h"

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT FaceModelAssImpFileHandlerFactory
{
public:
    // Create and return the file handler for the given extension or NULL
    // if not possible to get a file handler for given extension.
    static FaceModelAssImpFileHandler* make( const QString& ext);

    static void printAvailableFormats( std::ostream&);

private:
    static RModelIO::AssetImporter assimp;
};  // end class

}}   // end namespace

#endif
