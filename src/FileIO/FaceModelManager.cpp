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

#include <FileIO/FaceModelManager.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QFileInfo>
#include <QDebug>
#include <cassert>
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FileIO::FaceModelFileHandler;
using FaceTools::FileIO::FaceModelFileHandlerMap;
using FaceTools::FMS;
using FaceTools::FM;


size_t FaceModelManager::_loadLimit(0);
FaceModelFileHandlerMap FaceModelManager::_fhmap;
FMS FaceModelManager::_models;
std::unordered_map<FM*, QString> FaceModelManager::_mdata;
std::unordered_map<QString, FM*> FaceModelManager::_mfiles;    // Lookup models by current filepath
QString FaceModelManager::_err;


void FaceModelManager::setLoadLimit( size_t llimit) { _loadLimit = llimit;}


void FaceModelManager::add( FaceModelFileHandler* fii) { if ( fii) _fhmap.add(fii);}


bool FaceModelManager::hasPreferredFileFormat( const FM* fm)
{
    assert( fm != nullptr);
    assert( _models.count(const_cast<FM*>(fm)) > 0);
    return isPreferredFileFormat( _mdata.at(const_cast<FM*>(fm)));
}   // end hasPreferredFileFormat


bool FaceModelManager::isPreferredFileFormat( const QString& fname)
{
    const QString pext = _fhmap.preferredExt().toLower();
    const QString cext = QFileInfo( fname).suffix().toLower();
    return cext == pext;
}   // end isPreferredFileFormat


void FaceModelManager::_setModelFilepath( const FM* cfm, const QString& fname)
{
    FM* fm = const_cast<FM*>(cfm);
    _models.insert(fm);
    _mdata[fm] = fname;
    _mfiles[fname] = fm;
}   // end _setModelFilepath


bool FaceModelManager::write( const FM* cfm, QString *fpath)
{
    FM* fm = const_cast<FM*>(cfm);
    assert( _models.count(fm) > 0);
    QString savefilepath = _mdata.at(fm);
    QString delfilepath;    // Will not be empty if replacing filename
    if ( fpath)
    {
        if ( fpath->isEmpty())
            *fpath = savefilepath;
        else    // New filepath specified
        {
            delfilepath = savefilepath;
            savefilepath = *fpath;
        }   // end else
    }   // end if

    _err = "";  // Reset the error
    FaceModelFileHandler* fileio = _fhmap.writeInterface( savefilepath);
    if ( !fileio)
        _err = "File \"" + savefilepath + "\" is not an allowed file type!";
    else if ( !fileio->canWrite())
        _err = "Cannot write to " + fileio->getFileDescription() + " files!";
    else if ( !fileio->write( fm, savefilepath))
        _err = fileio->error();
    else    // Successful write
    {
        _mfiles.erase(delfilepath);
        _setModelFilepath( fm, savefilepath);
        fm->setModelSaved( fileio->canWriteTextures() || !fm->hasTexture());
        fm->setMetaSaved( isPreferredFileFormat(savefilepath) || !fm->hasMetaData());
    }   // end else

    return _err.isEmpty();
}   // end write


bool FaceModelManager::canWrite( const QString& fn)
{
    const QFileInfo finfo(fn);
    FaceModelFileHandler* fileio = _fhmap.writeInterface( finfo.filePath());
    return fileio && fileio->canWrite();
}   // end canWrite


bool FaceModelManager::canRead( const QString& fn)
{
    FaceModelFileHandler* fileio = nullptr;
    const QFileInfo finfo(fn);
    if ( finfo.exists())
        fileio = _fhmap.readInterface( finfo.filePath());
    return fileio && fileio->canRead();
}   // end canRead


bool FaceModelManager::hasFileHandler( const QString &fpath)
{
    return _fhmap.readInterface( fpath) != nullptr || _fhmap.writeInterface( fpath) != nullptr;
}   // end hasFileHandler


bool FaceModelManager::canSaveTextures( const QString &fpath)
{
    FaceModelFileHandler* fileio = _fhmap.writeInterface( fpath);
    return fileio && fileio->canWriteTextures();
}   // end canSaveTextures


bool FaceModelManager::isOpen( const QString& fn)
{
    const QFileInfo finfo(fn);
    return _mfiles.count( finfo.filePath()) > 0;
}   // end isOpen


FM* FaceModelManager::read( const QString& fn)
{
    const QFileInfo finfo(fn);
    const QString fname = finfo.filePath();

    _err = "";
    if ( _mfiles.count(fname) > 0)
    {
        _err = "File \"" + fname + "\" already open!";
        std::cerr << "Model already loaded!" << std::endl;
        return nullptr;
    }   // end if

    FaceModelFileHandler* fileio = nullptr;
    FM* fm = nullptr;
    if ( !finfo.exists())
        _err = "File \"" + fname + "\" does not exist!";
    else if ( (fileio = _fhmap.readInterface(fname)) == nullptr)
        _err = "File \"" + fname + "\" is not an allowed file type!";
    else if ( !fileio->canRead())
        _err = "Cannot read from " + fileio->getFileDescription() + " files!";
    else if ( (fm = fileio->read( fname)) == nullptr)
        _err = fileio->error();
    else
    {
        _setModelFilepath( fm, fname);
        fm->setModelSaved( true);
        fm->setMetaSaved( true);
    }   // end else

    return fm;
}   // end read


const QString& FaceModelManager::filepath( const FM* fm)
{
    assert( fm);
    assert( _models.count(const_cast<FM*>(fm)) > 0);
    return _mdata.at(const_cast<FM*>(fm));
}   // end filepath


FM* FaceModelManager::model( const QString& fname)
{
    FM* fm = nullptr;
    if ( _mfiles.count(fname) > 0)
        fm = _mfiles.at(fname);
    return fm;
}   // end model


void FaceModelManager::close( const FM* cfm)
{
    FM* fm = const_cast<FM*>(cfm);
    assert(_models.count(fm) > 0);
    _mfiles.erase(_mdata.at(fm));
    _models.erase(fm);
    _mdata.erase(fm);
    delete fm;
}   // end close


void FaceModelManager::printFormats( std::ostream& os)
{
    os << _fhmap << std::endl;
}   // end printFormats
