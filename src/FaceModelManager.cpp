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
#include <FaceModel.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <map>
using FaceTools::FaceModelManager;
using FaceTools::FileIOInterface;
using FaceTools::FaceModel;


QStringList createFilters( const boost::unordered_map<std::string, std::string>& fmap)
{
    typedef std::pair<std::string, std::string> FPair;
    std::vector<std::string> exts;
    foreach ( const FPair& fp, fmap)
        exts.push_back(fp.first);

    std::sort( exts.begin(), exts.end());   // Sort extensions alphanumerically
    QStringList filters;
    QString allfilters = "Any supported (";
    foreach ( const std::string& ext, exts)
    {
        filters << std::string( fmap.at(ext) + " (*." + ext + ")").c_str();
        allfilters.append( std::string("*." + ext + " ").c_str());
    }   // end foreach
    allfilters.remove(allfilters.size()-1, 1);  // Remove end space
    allfilters += ")";
    filters.insert(0, allfilters);
    return filters;
}   // end createFilters


FaceModelManager* FaceModelManager::s_fmm(NULL); // static

// private
FaceModelManager::FaceModelManager() {}

FaceModelManager::~FaceModelManager() {}

// public static
FaceModelManager& FaceModelManager::get()
{
    if ( !s_fmm)
        s_fmm = new FaceModelManager();
    return *s_fmm;
}   // end get


// public
bool FaceModelManager::load( const std::string& fname)
{
    if ( fname.empty())
        return false;

    std::string err;
    if ( !boost::filesystem::exists( fname))
        err = "File " + fname + " does not exist!";
    else if ( !isValidImportFilename(fname))
        err = "Invalid file type!";
    else
    {
        const std::string fext = FaceTools::getExtension(fname);
        FileIOInterface* fileio = _fileInterfaces.at(fext);
        assert( fileio->canImport());
        fileio->setFileOp( fname);
        fileio->process();
    }   // end else

    bool success = true;
    if ( !err.empty())
    {
        emit finishedImport( NULL, err.c_str());
        success = false;
    }   // end if
    return success;
}   // end load


// public
bool FaceModelManager::save( FaceModel* fmodel, const std::string& fname)
{
    if ( fname.empty())
        return false;

    bool success = false;
    if ( !isValidExportFilename( fname))
    {
        std::string err = fname + " has unsupported extension!";
        emit finishedExport( false, err.c_str());
    }   // end if
    else
    {
        const std::string fext = FaceTools::getExtension(fname);
        FileIOInterface* fileio = _fileInterfaces.at( fext);
        assert( fileio->canExport());
        fileio->setFileOp( fname, fmodel);
        fileio->process();
        success = true;
    }   // end else
    return success;
}   // end save


// private slot
void FaceModelManager::postProcessImport( FaceModel* fmodel, const QString& errmsg)
{
    if ( !fmodel)
    {
        std::cerr << "[ERROR] FaceTools::FaceModelManager::postProcessImport: " << errmsg.toStdString() << std::endl;
        emit finishedImport( NULL, errmsg);
        return;
    }   // end if

    const std::string filename = qobject_cast<FileIOInterface*>( sender())->getFilepath();
    fmodel->setSaveFilepath( filename);

    RFeatures::ObjModel::Ptr model = fmodel->getObjectMeta()->getObject();
    // Don't want models to have more than 1 texture map
    if ( model->getNumMaterials() > 1)
    {
        std::cerr << " =====[ Combining Textures ]=====" << std::endl;
        model->mergeMaterials();
    }   // end if

    // Clean the model - make a triangulated manifold.
    RFeatures::ObjModelIntegrityChecker ic( model);
    RFeatures::ObjModelIntegrityError ierror = ic.checkIntegrity();
    if ( ierror != RFeatures::NO_INTEGRITY_ERROR)
    {
        std::ostringstream oss;
        oss << "Model integrity error on load : CODE " << int(ierror);
        delete fmodel;
        fmodel = NULL;
        emit finishedImport( fmodel, oss.str().c_str());
        return;
    }   // end if

    if ( !ic.is2DManifold())
    {
        std::cerr << " =====[ Cleaning Model ]=====" << std::endl;
        FaceTools::clean(model);
        ic.checkIntegrity();
        if ( !ic.is2DManifold())
        {
            delete fmodel;
            emit finishedImport( NULL, "Failed to make model into a triangulated manifold!");
            return;
        }   // end if
    }   // end if

    fmodel->updateMesh(model);
    _fmodels.insert(fmodel);
    emit finishedImport( fmodel, errmsg);
}   // end postProcessImport


// public
bool FaceModelManager::close( FaceModel* fmodel)
{
    if ( _fmodels.count(fmodel) == 0)
        return false;

    emit closingModel( fmodel);
    _fmodels.erase(fmodel);
    delete fmodel;
    return true;
}   // end close


// public
void FaceModelManager::closeAll()
{
    while ( !_fmodels.empty())
        close( *_fmodels.begin());
}   // end closeAll


// public
bool FaceModelManager::isValidImportFilename( const std::string& fname)
{
    return _importExtDescMap.count( FaceTools::getExtension(fname)) > 0;
}   // end isValidImportFilename


// public
bool FaceModelManager::isValidExportFilename( const std::string& fname)
{
    return _exportExtDescMap.count( FaceTools::getExtension(fname)) > 0;
}   // end isValidExportFilename


// public
void FaceModelManager::printIOFormats( std::ostream& os)
{
    typedef std::pair<std::string, FileIOInterface*> FPair;
    foreach ( const FPair& fp, _fileInterfaces)
    {
        const FileIOInterface* fileio = fp.second;
        const std::string& desc = fileio->getFileDescription();
        if ( fileio->canImport())
            os << "LOAD ";
        else
            os << "     ";
        if ( fileio->canExport())
            os << "SAVE ";
        else
            os << "     ";
        os << desc << " (*." << fp.first << ")" << std::endl;
    }   // end foreach
}   // end printIOFormats


// public
QString FaceModelManager::getFilter( const std::string& ext)
{
    const std::string sxt = boost::algorithm::to_lower_copy(ext);
    const std::string desc = _fileInterfaces.at(sxt)->getFileDescription();
    return QString( std::string( desc + " (*." + sxt + ")").c_str());
}   // end getFilter


// public
void FaceModelManager::addFileFormat( FileIOInterface* fileio)
{
    assert(fileio);
    std::vector<std::string> exts;
    fileio->getFileExtensions( exts);
    foreach ( const std::string& ext, exts)
    {
        _fileInterfaces[ext] = fileio;
        if ( fileio->canImport())
            _importExtDescMap[ext] = fileio->getFileDescription();
        if ( fileio->canExport())
            _exportExtDescMap[ext] = fileio->getFileDescription();
        if ( _primaryExt.empty())
            _primaryExt = ext;
    }   // end foreach

    if ( fileio->canImport())
        connect( fileio, &FileIOInterface::finishedImport, this, &FaceModelManager::postProcessImport);
    if ( fileio->canExport())
        connect( fileio, &FileIOInterface::finishedExport, this, &FaceModelManager::finishedExport);
}   // end addFileFormat

