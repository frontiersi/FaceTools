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

#ifndef FACE_TOOLS_FILE_IO_INTERFACE_H
#define FACE_TOOLS_FILE_IO_INTERFACE_H

#include "FaceActionInterface.h"
#include <vector>

namespace FaceTools
{

class FaceModel;

class FaceTools_EXPORT FileIOInterface : public FaceAction
{ Q_OBJECT
public:
    FileIOInterface();
    virtual ~FileIOInterface(){}

    // Human readable file type description.
    virtual std::string getFileDescription() const = 0;

    // Get the file extensions into exts and return the number pushed in.
    // exts should NOT be cleared of its contents before pushing items to it!
    virtual size_t getFileExtensions( std::vector<std::string>& exts) const = 0;

    virtual bool canImport() const = 0;
    virtual bool canExport() const = 0;

    // For next import/export operation. Returns true iff can carry out the operation.
    // Defines an import operation by default. Carries out export if fmodel != NULL.
    // Returns true iff the operation is supported by the derived type.
    bool setFileOp( const std::string& fp, FaceModel* fmodel=NULL);

    // Return the filepath set in last call to setFileOp that returned true.
    const std::string getFilepath() const { return _filepath;}

signals:
    // On the result of an export operation, the FaceModel parameter will be NULL.
    // Whether NULL or not, a non-empty string denotes an error of some sort.
    // After returning from the listening function, filepath (and model) are reset.
    void finished( FaceModel*, const std::string&);

protected:
    std::string _err;       // Set if error

    // Re-implement load and/or save functions if available
    virtual FaceModel* load( const std::string&) { return NULL;}
    virtual bool save( const FaceModel*, const std::string&) { return false;}

    virtual bool doAction();

private slots:
    void doFinished(bool);

private:
    FaceModel* _fmodel;
    std::string _filepath;  // Set by client for next load/save operation.
    bool _doExport;
};  // end class

}   // end namespace

#endif
