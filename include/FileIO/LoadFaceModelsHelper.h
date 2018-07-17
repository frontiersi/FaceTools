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

#ifndef FACE_TOOLS_FILE_IO_LOAD_FACE_MODELS_HELPER_H
#define FACE_TOOLS_FILE_IO_LOAD_FACE_MODELS_HELPER_H

#include <FaceTools_Export.h>
#include <Hashing.h>
#include <QWidget>
#include <QStringList>
#include <unordered_map>

namespace FaceTools {
class FaceModel;
namespace FileIO {
class FaceModelManager;

class FaceTools_EXPORT LoadFaceModelsHelper
{
public:
    // Filters the list of filenames into only those that can be loaded and presents
    // warnings for others if the filetypes aren't supported. If the size of the given
    // list when added to the number of already opened models is larger than the
    // load limit then the list is cleared (and this function returns zero).
    // Returns number of filenames left in list and set in _filenames;
    size_t setFilteredFilenames( const QStringList& filenames);

    // Loads the models set by the last call to setFilteredFilenames and returns
    // the number successfully loaded. Load errors can be displayed afterwards using
    // showLoadErrors(). Every time a model is successfuly loaded, its filepath is
    // appended to the vector returned from lastLoaded - which is reset before
    // every call to this function. The internal list of filenames to load is
    // cleared before this function returns.
    size_t loadModels();

    // Convenience function to load a single model from filename.
    bool loadModel( const QString& filename);

    // Returns the filepaths of the models last successfully loaded.
    const std::vector<std::string>& lastLoaded() const { return _loaded;}

    // Shows the last set of load errors and then clears these errors.
    void showLoadErrors();

    // Returns true if no more models may be loaded (according to the FaceModelManager).
    bool reachedLoadLimit() const;

    // Return parent widget for client to make custom dialogs.
    QWidget* parentWidget() const { return _parent;}

    QString createImportFilters() const;
    QStringList createSimpleImportFilters() const;

private:
    FaceModelManager *_fmm;
    QWidget *_parent;
    QStringList _filenames;
    std::vector<std::string> _loaded;
    std::unordered_map<QString, QStringList> _failnames;

    LoadFaceModelsHelper( FaceModelManager* fmm, QWidget *parent);
    ~LoadFaceModelsHelper(){}
    LoadFaceModelsHelper( const LoadFaceModelsHelper&) = delete;
    void operator=( const LoadFaceModelsHelper&) = delete;
    friend class FaceModelManager;
};  // end class

}   // end namespace
}   // end namespace

#endif
