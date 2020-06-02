/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_FILE_IO_ASYNC_MODEL_LOADER_H
#define FACE_TOOLS_FILE_IO_ASYNC_MODEL_LOADER_H

#include "FaceModelManager.h"
#include <QThread>

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT AsyncModelLoader : public QThread
{ Q_OBJECT
public:
    // Provide absolute filepath to model to load.
    // Can be any format currently being accepted by the FaceModelManager.
    explicit AsyncModelLoader( const std::string& fpath);

signals:
    void loadedModel( FM*);

protected:
    void run() override;

private:
    const std::string _fpath;
};  // end class

}}   // end namespace

#endif
