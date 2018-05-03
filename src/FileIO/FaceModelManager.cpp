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

#include <FaceModelManager.h>
#include <MiscFunctions.h>
#include <FaceTools.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
using FaceTools::FileIO::FaceModelFileHandler;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceModel;


namespace {

std::string postProcess( RFeatures::ObjModel::Ptr model)
{
    // Don't want models to have more than 1 texture map
    if ( model->getNumMaterials() > 1)
    {
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelManager::read: Combining Textures" << std::endl;
        model->mergeMaterials();
    }   // end if

    // Clean the model - make a triangulated manifold.
    RFeatures::ObjModelIntegrityChecker ic( model);
    if ( !ic.checkIntegrity())
        return "Loaded model has internally invalid geometry!";
    else if ( !ic.is2DManifold())
    {
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelManager::read: Cleaning Model" << std::endl;
        FaceTools::clean(model);
        ic.checkIntegrity();
        if ( !ic.is2DManifold())
            return "Unable to generate a triangulated manifold(s) from the loaded model!";
    }   // end if
    return "";
}   // end postProcess

}   // end namespace


// public
FaceModelManager::FaceModelManager( FaceModelFileHandler* fii, size_t llimit)
    : _loadLimit(llimit)
{
    add(fii);   // First added is preferred
}  // end ctor


// public
void FaceModelManager::add( FaceModelFileHandler* fii) { _fhmap.add(fii);}


// public
bool FaceModelManager::hasPreferredFileFormat( FaceModel* fm) const
{
    assert( _models.count(fm) > 0);
    const QString& pext = _fhmap.getPreferredExt();
    const std::string cext = FaceTools::getExtension( _mdata.at(fm).filepath);
    return cext == pext.toLower().toStdString();
}   // end hasPreferredFileFormat


// private
void FaceModelManager::setModelData( FaceModel* fm, const std::string& fname)
{
    _models.insert(fm);
    _mdata[fm].filepath = fname;
    _mdata[fm].saved = true;
    _mfiles[fname] = fm;
}   // end setModelData


// public
bool FaceModelManager::write( FaceModel* fm, std::string* fpath)
{
    assert( _models.count(fm) > 0);
    std::string savefilepath = _mdata.at(fm).filepath;
    if ( fpath)
    {
        if (!fpath->empty())
            savefilepath = *fpath;  // New filepath specified
        else
            *fpath = savefilepath;
    }   // end if

    _err = "";  // Reset the error
    FaceModelFileHandler* fileio = _fhmap.getSaveInterface( savefilepath);
    if ( !fileio)
        _err = "File \"" + savefilepath + "\" is not an allowed file type!";
    else if ( !fileio->canWrite())
        _err = "Cannot write to " + fileio->getFileDescription().toStdString() + " files!";
    else if ( !fileio->write( fm, QString(savefilepath.c_str())))
        _err = fileio->error().toStdString();
    else    // Successful write
        setModelData( fm, savefilepath);

    return _err.empty();
}   // end save


// public
bool FaceModelManager::canRead( const std::string& fname) const
{
    if ( !boost::filesystem::exists( fname))
       return false;
    FaceModelFileHandler* fileio = _fhmap.getLoadInterface( fname);
    return fileio && fileio->canRead();
}   // end canRead


// public
FaceModel* FaceModelManager::read( const std::string& fname)
{
    assert( !fname.empty());
    std::string err;
    if ( _mfiles.count(fname) > 0)
    {
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelManager::read: Model already loaded!" << std::endl;
        return _mfiles.at(fname);
    }   // end if

    FaceModelFileHandler* fileio = NULL;
    FaceModel* fm = NULL;
    _err = "";
    if ( !boost::filesystem::exists( fname))
        _err = "File \"" + fname + "\" does not exist!";
    else if ( (fileio = _fhmap.getLoadInterface(fname)) == NULL)
        _err = "File \"" + fname + "\" is not an allowed file type!";
    else if ( !fileio->canRead())
        _err = "Cannot read from " + fileio->getFileDescription().toStdString() + " files!";
    else if ( (fm = fileio->read( QString( fname.c_str()))) == NULL)
        _err = fileio->error().toStdString();
    else if (!( _err = postProcess( fm->model())).empty())
    {
        delete fm;
        fm = NULL;
    }   // end else if
    else
        setModelData( fm, fname);

    return fm;
}   // end read


// public
bool FaceModelManager::isSaved( FaceModel* fm) const
{
    assert(_models.count(fm) > 0);
    return _mdata.at(fm).saved;
}   // end isSaved


// public
void FaceModelManager::setUnsaved( FaceModel* fm)
{
    assert(_models.count(fm) > 0);
    _mdata.at(fm).saved = false;
}   // end setUnsaved


// public
const std::string& FaceModelManager::filepath( FaceModel* fm) const
{
    assert(_models.count(fm) > 0);
    return _mdata.at(fm).filepath;
}   // end filepath


// public
void FaceModelManager::close( FaceModel* fm)
{
    assert(_models.count(fm) > 0);
    if ( !isSaved(fm))
        std::cerr << "[WARNING] FaceTools::FileIO::FaceModelManager::close: Model is unsaved!" << std::endl;
    _mfiles.erase(_mdata.at(fm).filepath);
    _models.erase(fm);
    _mdata.erase(fm);
    delete fm;
}   // end close


// public
void FaceModelManager::printFormats( std::ostream& os) const
{
    os << _fhmap << std::endl;
}   // end printFormats
