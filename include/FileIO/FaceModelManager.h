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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_MANAGER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_MANAGER_H

/**
 * Allows clients to read/write FaceModel instances in a file format independent way
 * (using FaceModelFileHandlerMap). Additionally, generates hashes of FaceModel instances
 * on save to track whether save is necessary for a FaceModel instance.
 */

#include "FaceModelFileHandlerMap.h"

namespace FaceTools {
namespace FileIO {

class FaceTools_EXPORT FaceModelManager
{
public:
    // Construct with a single FaceModelFileHandler to specify the preferred file format.
    // The loadLimit specifies the maximum number of models that can be open at once.
    FaceModelManager( FaceModelFileHandler*, size_t loadLimit=UINT_MAX);

    // Add other file formats (forwards to FaceModelFileHandlerMap).
    void add( FaceModelFileHandler*);

    // Returns true iff given model is currently saved in the preferred file format (according to FaceModelFileHandlerMap).
    bool hasPreferredFileFormat( FaceModel*) const;

    // Return the file formats map for creating import/export filters etc.
    const FaceModelFileHandlerMap& fileFormats() const { return _fhmap;}

    // Save model to filepath (returns true on success).
    // If fpath NULL try to save over the current file path associated with the model.
    // If fpath not NULL but empty, try to save using the currently stored file path, and copy into fpath on return.
    // If fpath points to a non-empty string, try to save to this new location and update the model's stored file path.
    // Generates and stores the model hash upon success.
    bool write( FaceModel*, std::string* fpath=NULL);

    // Returns true iff the file at given path can be read in.
    bool canRead( const std::string&) const;

    // Load in a model (returning NULL on fail). If model at given path already present, simply return it.
    // Also causes signal loadedModel() to fire.
    FaceModel* read( const std::string&);

    bool isSaved( FaceModel*) const;
    void setUnsaved( FaceModel*);

    // Get the nature of the error if read returns NULL or write returns false.
    const std::string& error() const { return _err;}

    // Return the filepath for the model.
    const std::string& filepath( FaceModel*) const;

    // Close given model and release memory (client must check if saved!).
    // Also causes closingModel to fire just prior to actually closing.
    void close( FaceModel*);

    // Returns the number of models currently open.
    size_t numOpen() const { return _mdata.size();}
    size_t loadLimit() const { return _loadLimit;}  // Load limit not enforced by FaceModelManager (clients must do this)

    // Get the complete set of models currently open.
    const std::unordered_set<FaceModel*>& opened() const { return _models;}

    void printFormats( std::ostream&) const;    // Prints the accepted file formats

private:
    const size_t _loadLimit;
    FaceModelFileHandlerMap _fhmap;
    struct ModelData
    {
        std::string filepath;
        bool saved;
    };  // end struct
    std::unordered_set<FaceModel*> _models;
    std::unordered_map<FaceModel*, ModelData> _mdata;
    std::unordered_map<std::string, FaceModel*> _mfiles;    // Lookup models by current filepath
    std::string _err;
    void setModelData( FaceModel*, const std::string&);

    FaceModelManager( const FaceModelManager&);
    void operator=( const FaceModelManager&);
};  // end class

}   // end namespace
}   // end namespace

#endif
