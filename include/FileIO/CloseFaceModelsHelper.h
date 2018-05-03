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

#ifndef FACE_TOOLS_FILE_IO_CLOSE_FACE_MODELS_HELPER_H
#define FACE_TOOLS_FILE_IO_CLOSE_FACE_MODELS_HELPER_H

#include <QWidget>
#include <QStringList>
#include "FaceModelManager.h"

namespace FaceTools {
namespace FileIO {

class FaceTools_EXPORT CloseFaceModelsHelper : public QObject
{ Q_OBJECT
public:
    CloseFaceModelsHelper( FaceModelManager* fmm, QWidget *parent);

    QWidget* parentWidget() const { return _parent;}    // Parent widget for client to make custom dialogs.
    const std::unordered_set<FaceModel*>& opened() const { return _fmm->opened();}  // The open models
    const std::string& filepath( FaceModel *fm) const { return _fmm->filepath(fm);} // Filepath for given model
    bool isSaved( FaceModel* fm) const { return _fmm->isSaved(fm);}     // Whether model has been saved

    void close( FaceModel* fm);  // Close the given model
    void close( const std::unordered_set<FaceModel*>& fset);    // Close a bunch

signals:
    void closingModel( FaceModel*); // Emitted just prior to closing the model.

private:
    FaceModelManager *_fmm;
    QWidget *_parent;
};  // end class

}   // end namespace
}   // end namespace

#endif
