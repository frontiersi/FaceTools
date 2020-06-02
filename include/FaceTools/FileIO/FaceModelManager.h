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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_MANAGER_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_MANAGER_H

#include "FaceModelFileHandlerMap.h"

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT FaceModelManager
{
public:
    // The loadLimit specifies the maximum number of models that can be open at once (initially zero).
    static void setLoadLimit( size_t loadLimit);

    // Add file formats (forwards to FaceModelFileHandlerMap).
    // The first added file format is the preferred file format.
    static void add( FaceModelFileHandler*);

    // Returns true iff given model is currently saved in the preferred file format (according to FaceModelFileHandlerMap).
    static bool hasPreferredFileFormat( const FM*);

    // Does the given filename have the extension of the preferred file format?
    static bool isPreferredFileFormat( const std::string&);

    // Return the file formats map for creating import/export filters etc.
    static const FaceModelFileHandlerMap& fileFormats() { return _fhmap;}

    // Save model to filepath (returns true on success).
    // If fpath null try to save over the current file path associated with the model.
    // If fpath not null but empty, try to save using the currently stored file path, and copy into fpath on return.
    // If fpath points to a non-empty string, try to save to this new location and update the model's stored file path.
    // Generates and stores the model hash upon success.
    static bool write( const FM*, std::string* fpath=nullptr);

    // Returns true iff the file at given path can be read in.
    static bool canRead( const std::string&);

    // Returns true iff the given filepath can be written to using an available file format handler.
    static bool canWrite( const std::string&);

    // Returns true iff the given filepath can save textures (looks at the extension).
    static bool canSaveTextures( const std::string&);

    // Returns true iff there is a file hander for the given file (looks at the extension).
    // (Ignores whether the file exists or not).
    static bool hasFileHandler( const std::string&);

    // Returns true iff the file matching the given filepath is already open.
    static bool isOpen( const std::string&);

    // Load in a model (returning null on fail). Also returns null if model already open.
    static FM* read( const std::string&);

    // Get the nature of the error if read returns null or write returns false.
    static const std::string& error() { return _err;}

    // Return the filepath for the model.
    static const std::string& filepath( const FM*);

    // Return the open model for the given filepath or null if not open.
    static FM* model( const std::string&);

    // Close given model and release memory (client must check if saved!).
    static void close( const FM*);

    // Returns the number of models currently open.
    static size_t numOpen() { return _mdata.size();}
    static size_t loadLimit() { return _loadLimit;}  // Load limit not enforced by FaceModelManager (clients must do this)

    // Get the complete set of models currently open.
    static const FMS& opened() { return _models;}

    static void printFormats( std::ostream&);    // Prints the accepted file formats

private:
    static size_t _loadLimit;
    static FaceModelFileHandlerMap _fhmap;
    static FMS _models;
    static std::unordered_map<FM*, std::string> _mdata;
    static std::unordered_map<std::string, FM*> _mfiles;    // Lookup models by current filepath
    static std::string _err;
    static void _setModelFilepath( const FM*, const std::string&);
};  // end class

using FMM = FaceModelManager;

}}   // end namespaces

#endif
