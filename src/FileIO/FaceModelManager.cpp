/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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
#include <FileIO/FaceModelDatabase.h>
#include <FileIO/FaceModelXMLFileHandler.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QFileInfo>
#include <QDebug>
#include <cassert>
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FileIO::FaceModelFileHandler;
using FaceTools::FileIO::FaceModelFileHandlerMap;
using FMD = FaceTools::FileIO::FaceModelDatabase;
using FaceTools::FMS;
using FaceTools::FM;


size_t FaceModelManager::_loadLimit(2);
FaceModelFileHandlerMap FaceModelManager::_fhmap;
FMS FaceModelManager::_models;
std::unordered_map<FM*, QString> FaceModelManager::_mpaths;
std::unordered_map<QString, FM*> FaceModelManager::_mfiles;    // Lookup models by current filepath
QString FaceModelManager::_err;


void FaceModelManager::add( FaceModelFileHandler* fii) { if ( fii) _fhmap.add(fii);}


bool FaceModelManager::hasPreferredFileFormat( const FM &fm)
{
    return isPreferredFileFormat( _mpaths.at(const_cast<FM*>(&fm)));
}   // end hasPreferredFileFormat


bool FaceModelManager::isPreferredFileFormat( const QString& fname)
{
    const QString pext = _fhmap.preferredExt().toLower();
    const QString cext = QFileInfo( fname).suffix().toLower();
    return cext == pext;
}   // end isPreferredFileFormat


void FaceModelManager::_setModelFilepath( const FM &cfm, const QString& fpath)
{
    FM* fm = const_cast<FM*>(&cfm);
    _models.insert(fm);
    _mpaths[fm] = fpath;
    _mfiles[fpath] = fm;
}   // end _setModelFilepath


bool FaceModelManager::write( const FM &cfm, QString &fpath)
{
    FM* fm = const_cast<FM*>(&cfm);
    assert( _models.count(fm) > 0);
    const QString prvfilepath = _mpaths.at(fm);
    QString savefilepath = prvfilepath;
    if ( fpath.isEmpty())
        fpath = savefilepath;
    else    // New filepath specified
        savefilepath = fpath;

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
        _mfiles.erase(prvfilepath);
        fm->setModelSaved( fileio->canWriteTextures() || !fm->hasTexture());
        fm->setMetaSaved( isPreferredFileFormat(savefilepath) || !fm->hasMetaData());
        _setModelFilepath( *fm, savefilepath);
        const bool modelMetaAuth = isPreferredFileFormat(savefilepath) && fm->hasMetaData() && !fm->subjectId().isEmpty();
        FMD::refreshImage( *fm, savefilepath, prvfilepath, modelMetaAuth);
    }   // end else

    return _err.isEmpty();
}   // end write


bool FaceModelManager::canWrite( const QString& fn)
{
    const QFileInfo finfo(fn);
    FaceModelFileHandler* fileio = _fhmap.writeInterface( finfo.absoluteFilePath());
    return fileio && fileio->canWrite();
}   // end canWrite


bool FaceModelManager::canRead( const QString& fn)
{
    FaceModelFileHandler* fileio = nullptr;
    const QFileInfo finfo(fn);
    if ( finfo.exists())
        fileio = _fhmap.readInterface( finfo.absoluteFilePath());
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
    return _mfiles.count( finfo.absoluteFilePath()) > 0;
}   // end isOpen


FM* FaceModelManager::read( const QString& fn)
{
    const QFileInfo finfo(fn);
    const QString fpath = finfo.absoluteFilePath();

    _err = "";
    if ( _mfiles.count(fpath) > 0)
    {
        _err = "File \"" + fpath + "\" already open!";
        std::cerr << "Model already loaded!" << std::endl;
        return nullptr;
    }   // end if

    FaceModelFileHandler* fileio = nullptr;
    FM* fm = nullptr;
    if ( !finfo.exists())
        _err = "File \"" + fpath + "\" does not exist!";
    else if ( (fileio = _fhmap.readInterface(fpath)) == nullptr)
        _err = "File \"" + fpath + "\" is not an allowed file type!";
    else if ( !fileio->canRead())
        _err = "Cannot read from " + fileio->getFileDescription() + " files!";
    else if ( (fm = fileio->read( fpath)) == nullptr)
        _err = fileio->error();
    else
    {
        fm->setModelSaved( true);
        fm->setMetaSaved( true);
        _setModelFilepath( *fm, fpath);
    }   // end else

    return fm;
}   // end read


QString FaceModelManager::filepath( const FM &fm)
{
    QString fpath;
    FM *tmp = const_cast<FM*>(&fm);
    if ( _mpaths.count(tmp) > 0)
        fpath = _mpaths.at(tmp);
    return fpath;
}   // end filepath


FM* FaceModelManager::model( const QString& fpath)
{
    FM* fm = nullptr;
    if ( _mfiles.count(fpath) > 0)
        fm = _mfiles.at(fpath);
    return fm;
}   // end model


void FaceModelManager::close( const FM &cfm)
{
    FM* fm = const_cast<FM*>(&cfm);
    assert(_models.count(fm) > 0);
    const QString fpath = _mpaths.at(fm);
    _mfiles.erase(fpath);
    _mpaths.erase(fm);
    _models.erase(fm);
    delete fm;
}   // end close


FM *FaceModelManager::other( const FM &ifm)
{
    assert( _loadLimit == 2);
    FM *ofm = nullptr;
    if ( _models.size() == 2)
    {
        FM *fm0 = *_models.begin();
        FM *fm1 = *++_models.begin();
        const FM *pfm = &ifm;
        if ( fm0 == pfm)
            ofm = fm1;
        else if ( fm1 == pfm)
            ofm = fm0;
    }   // end if
    return ofm;
}   // end other


void FaceModelManager::printFormats( std::ostream& os)
{
    os << _fhmap << std::endl;
}   // end printFormats

