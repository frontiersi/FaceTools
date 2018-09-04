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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_MANAGER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_MANAGER_H

/**
 * Allows clients to read/write FaceModel instances in a file format independent way
 * (using FaceModelFileHandlerMap). Additionally, generates hashes of FaceModel instances
 * on save to track whether save is necessary for a FaceModel instance.
 */

#include "FaceModelFileHandlerMap.h"
#include "LoadFaceModelsHelper.h"

namespace FaceTools {
namespace FileIO {

class FaceTools_EXPORT FaceModelManager
{
public:
    // The loadLimit specifies the maximum number of models that can be open at once.
    FaceModelManager( QWidget* parent, size_t loadLimit=UINT_MAX);
    virtual ~FaceModelManager();

    // Use the loader delegate interface to simplify loading operations.
    LoadFaceModelsHelper* loader() { return _loader;}

    // Add file formats (forwards to FaceModelFileHandlerMap).
    // The first added file format is the preferred file format.
    void add( FaceModelFileHandler*);

    // Returns true iff given model is currently saved in the preferred file format (according to FaceModelFileHandlerMap).
    bool hasPreferredFileFormat( FM*) const;

    // Does the given filename have the extension of the preferred file format?
    bool isPreferredFileFormat( const std::string&) const;

    // Return the file formats map for creating import/export filters etc.
    const FaceModelFileHandlerMap& fileFormats() const { return _fhmap;}

    // Save model to filepath (returns true on success).
    // If fpath null try to save over the current file path associated with the model.
    // If fpath not null but empty, try to save using the currently stored file path, and copy into fpath on return.
    // If fpath points to a non-empty string, try to save to this new location and update the model's stored file path.
    // Generates and stores the model hash upon success.
    bool write( FM*, std::string* fpath=nullptr);

    // Returns true iff the file at given path can be read in.
    bool canRead( const std::string&) const;

    // Returns true iff the file matching the given filepath is already open.
    bool isOpen( const std::string&) const;

    // Load in a model (returning null on fail). Also returns null if model already open.
    // Also causes signal loadedModel() to fire.
    FM* read( const std::string&);

    // Get the nature of the error if read returns null or write returns false.
    const std::string& error() const { return _err;}

    // Return the filepath for the model.
    const std::string& filepath( FM*) const;

    // Return the open model for the given filepath or null if not open.
    FM* model( const std::string&) const;

    // Close given model and release memory (client must check if saved!).
    void close( FM*);

    // Returns the number of models currently open.
    size_t numOpen() const { return _mdata.size();}
    size_t loadLimit() const { return _loadLimit;}  // Load limit not enforced by FaceModelManager (clients must do this)

    // Get the complete set of models currently open.
    const FaceModelSet& opened() const { return _models;}

    void printFormats( std::ostream&) const;    // Prints the accepted file formats

private:
    LoadFaceModelsHelper *_loader;
    const size_t _loadLimit;
    FaceModelFileHandlerMap _fhmap;
    FaceModelSet _models;
    std::unordered_map<FM*, std::string> _mdata;
    std::unordered_map<std::string, FM*> _mfiles;    // Lookup models by current filepath
    std::string _err;
    void setModelFilepath( FM*, const std::string&);

    FaceModelManager( const FaceModelManager&) = delete;
    void operator=( const FaceModelManager&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
