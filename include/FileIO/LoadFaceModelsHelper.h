/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <FaceTypes.h>
#include <FaceViewSet.h>
#include <QWidget>

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT LoadFaceModelsHelper
{
public:
    explicit LoadFaceModelsHelper( QWidget* parent=nullptr);

    // Filters the list of filenames into only those that can be loaded and presents
    // warnings for others if the filetypes aren't supported. If the size of the given
    // list when added to the number of already opened models is larger than the
    // load limit then the list is cleared (and this function returns zero).
    // Returns number of filenames left in list and set in _filenames;
    int setFilteredFilenames( const QStringList& filenames);

    // Returns the list of filenames to attempt to load in the next call to loadModels.
    const QStringList& filenames() const { return _filenames;}

    // Loads the models set by the last call to setFilteredFilenames and returns the number
    // successfully loaded. Load errors can be displayed afterwards using showLoadErrors().
    // Call lastLoaded() to return the set of loaded models as newly created FaceViews.
    // The internal list of filenames to load is cleared before this function returns.
    size_t loadModels();

    // Convenience function to load a single model from filename.
    bool loadModel( const QString& filename);

    // Returns the set of just loaded models.
    const FMS& lastLoaded() const { return _loaded;}

    // Shows the last set of load errors and then clears these errors.
    void showLoadErrors();

    QWidget *parent() const { return _parent;}

private:
    QWidget *_parent;
    QStringList _filenames;
    std::unordered_map<QString, QStringList> _failnames;
    FMS _loaded;

    LoadFaceModelsHelper( const LoadFaceModelsHelper&) = delete;
    void operator=( const LoadFaceModelsHelper&) = delete;
};  // end class

}}   // end namespace

#endif
