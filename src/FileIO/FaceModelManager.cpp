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

#include <FaceModelManager.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FileIO.h> // rlib
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FileIO::FaceModelFileHandler;
using FaceTools::FileIO::FaceModelFileHandlerMap;
using FaceTools::FMS;
using FaceTools::FM;


size_t FaceModelManager::_loadLimit(0);
FaceModelFileHandlerMap FaceModelManager::_fhmap;
FMS FaceModelManager::_models;
std::unordered_map<FM*, std::string> FaceModelManager::_mdata;
std::unordered_map<std::string, FM*> FaceModelManager::_mfiles;    // Lookup models by current filepath
std::string FaceModelManager::_err;


void FaceModelManager::setLoadLimit( size_t llimit) { _loadLimit = llimit;}


void FaceModelManager::add( FaceModelFileHandler* fii) { if ( fii) _fhmap.add(fii);}


bool FaceModelManager::hasPreferredFileFormat( FM* fm)
{
    assert( fm != nullptr);
    assert( _models.count(fm) > 0);
    return isPreferredFileFormat( _mdata.at(fm));
}   // end hasPreferredFileFormat


bool FaceModelManager::isPreferredFileFormat( const std::string& fname)
{
    const QString& pext = _fhmap.preferredExt();
    const std::string cext = rlib::getExtension( fname);
    return cext == pext.toLower().toStdString();
}   // end isPreferredFileFormat


// private
void FaceModelManager::setModelFilepath( FM* fm, const std::string& fname)
{
    _models.insert(fm);
    _mdata[fm] = fname;
    _mfiles[fname] = fm;
    fm->setSaved();
}   // end setModelFilepath


bool FaceModelManager::write( FM* fm, std::string* fpath)
{
    assert( _models.count(fm) > 0);
    std::string savefilepath = _mdata.at(fm);
    std::string delfilepath;    // Will not be empty if replacing filename
    if ( fpath)
    {
        if ( fpath->empty())
            *fpath = savefilepath;
        else    // New filepath specified
        {
            delfilepath = savefilepath;
            savefilepath = *fpath;
        }   // end else
    }   // end if

    std::cerr << "[INFO] FaceTools::FileIO::FaceModelManager::write: Writing out file \"" << savefilepath << "\"" << std::endl;

    _err = "";  // Reset the error
    FaceModelFileHandler* fileio = _fhmap.getSaveInterface( savefilepath);
    fm->lockForRead();
    if ( !fileio)
        _err = "File \"" + savefilepath + "\" is not an allowed file type!";
    else if ( !fileio->canWrite())
        _err = "Cannot write to " + fileio->getFileDescription().toStdString() + " files!";
    else if ( !fileio->write( fm, QString(savefilepath.c_str())))
        _err = fileio->error().toStdString();
    else    // Successful write
    {
        _mfiles.erase(delfilepath);
        setModelFilepath( fm, savefilepath);
    }   // end else
    fm->unlock();

    return _err.empty();
}   // end save


bool FaceModelManager::canRead( const std::string& fname)
{
    if ( !boost::filesystem::exists( fname))
       return false;
    FaceModelFileHandler* fileio = _fhmap.getLoadInterface( fname);
    return fileio && fileio->canRead();
}   // end canRead


bool FaceModelManager::isOpen( const std::string& fname) { return _mfiles.count(fname) > 0;}


FM* FaceModelManager::read( const std::string& fname)
{
    assert( !fname.empty());

    _err = "";
    if ( _mfiles.count(fname) > 0)
    {
        _err = "File \"" + fname + "\" already open!";
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelManager::read: Model already loaded!" << std::endl;
        return nullptr;
    }   // end if

    std::cerr << "[INFO] FaceTools::FileIO::FaceModelManager::read: Reading in file \"" << fname << "\"" << std::endl;

    FaceModelFileHandler* fileio = nullptr;
    FM* fm = nullptr;
    if ( !boost::filesystem::exists( fname))
        _err = "File \"" + fname + "\" does not exist!";
    else if ( (fileio = _fhmap.getLoadInterface(fname)) == nullptr)
        _err = "File \"" + fname + "\" is not an allowed file type!";
    else if ( !fileio->canRead())
        _err = "Cannot read from " + fileio->getFileDescription().toStdString() + " files!";
    else if ( (fm = fileio->read( QString( fname.c_str()))) == nullptr)
        _err = fileio->error().toStdString();
    else
        setModelFilepath( fm, fname);

    return fm;
}   // end read


const std::string& FaceModelManager::filepath( const FM* fm)
{
    assert(_models.count(const_cast<FM*>(fm)) > 0);
    return _mdata.at(const_cast<FM*>(fm));
}   // end filepath


FM* FaceModelManager::model( const std::string& fname)
{
    FM* fm = nullptr;
    if ( _mfiles.count(fname) > 0)
        fm = _mfiles.at(fname);
    return fm;
}   // end model


void FaceModelManager::close( FM* fm)
{
    assert(_models.count(fm) > 0);
    if ( !fm->isSaved())
        std::cerr << "[WARNING] FaceTools::FileIO::FaceModelManager::close: Model is unsaved!" << std::endl;
    std::cerr << "[INFO] FaceTools::FileIO::FaceModelManager::close: closing model " << fm << std::endl;
    _mfiles.erase(_mdata.at(fm));
    _models.erase(fm);
    _mdata.erase(fm);
    delete fm;
}   // end close


void FaceModelManager::printFormats( std::ostream& os)
{
    os << _fhmap << std::endl;
}   // end printFormats
