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

#include <FileIOInterface.h>
using FaceTools::FileIOInterface;
using FaceTools::FaceModel;


FileIOInterface::FileIOInterface()
    : FaceAction(), _err(""), _fmodel(NULL), _filepath(""), _doExport(false)
{
    setAsync(true);
    connect( this, &FileIOInterface::finished, this, &FileIOInterface::doFinished);
    setEnabled(false);
}   // end ctor


// public
bool FileIOInterface::setFileOp( const std::string& fp, FaceModel* fmodel)
{
    if (( !canImport() && !fmodel) || ( !canExport() && fmodel))
        return false;

    _filepath = fp;
    _fmodel = fmodel;
    _doExport = _fmodel != NULL;
    _err = "";
    setEnabled(true);
    return true;
}   // end setFileOp


// private slot
void FileIOInterface::doFinished()
{
    setEnabled(false);
    if ( _doExport)
        emit finishedExport();
    else
        emit finishedImport();
    _fmodel = NULL;
    _filepath = "";
    _err = "";
    _doExport = false;
}   // end doFinished


// protected
bool FileIOInterface::doAction()
{
    bool success;
    if ( _doExport)
        success = save( _fmodel, _filepath);
    else
    {
        _fmodel = load( _filepath);
        success = _fmodel != NULL;
    }   // end if
    return success;
}   // end doAction
