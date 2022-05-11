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

#ifndef FACE_TOOLS_FILE_IO_BULK_METADATA_READER_H
#define FACE_TOOLS_FILE_IO_BULK_METADATA_READER_H

#include "FaceModelManager.h"
#include <QFileInfoList>
#include <QTemporaryDir>
#include <QThread>

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT BulkMetadataReader : public QThread
{ Q_OBJECT
public:
    /**
     * Load metadata from the given list of 3DF files (optionally loading
     * saved thumbnail images too if withThumbs true) and emit onLoadedModels
     * when finished.
     */
    BulkMetadataReader( const QFileInfoList&, bool withThumbs=false);

    // Run in calling thread. Returns models with matching number of entries to input QFileInfoList.
    QList<FaceTools::FM*> loadModels() const;

signals:
    // Upon emitting, the lists have the same length. If a model failed to load,
    // it is null. Models should be deleted after caller is finished with them.
    void onLoadedModels( QFileInfoList, QList<FaceTools::FM*>);

protected:
    void run() override;

private:
    const QFileInfoList _files;
    const bool _withThumbs;
    QTemporaryDir _tdir;
};  // end class

}}   // end namespaces
     
#endif
