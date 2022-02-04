/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_DATABASE_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_DATABASE_H

#include "FaceModelFileData.h"
#include <QFileInfo>
#include <QMutex>

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT FaceModelDatabase
{
public:
    // Initialise the database by creating the initial connection and defining the schema.
    // Not thread safe!
    static bool init();

    // Clear all data and reset to empty. Thread safe.
    static void reset();

    // Refresh the given path for 3DF files. The path may be for a single file or
    // a directory. If a directory, only the top level contents is parsed.
    // Returns the number of 3DF files found or -1 if parsing failed for any
    // of the files. This function blocks.
    static int refresh( const QString &path);

private:
    static bool _isInit;
    static QMutex _lock;
    static bool _addImage( const QFileInfo&);
};  // end class

}}   // end namespaces

#endif
