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
#include <FileIOInterface.h>
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


class FaceModelManager::Deleter
{ public:
    void operator()( const FaceModelManager* fmm) { delete fmm;}
};  // end class

FaceModelManager::Ptr FaceModelManager::s_fmm;

// private static
FaceModelManager::Ptr FaceModelManager::get()
{
    if ( !s_fmm)
        s_fmm = Ptr( new FaceModelManager, Deleter());
    return s_fmm;
}   // end get


// private
FaceModelManager::FaceModelManager()
{
    std::cerr << "=====[ FaceModelManager Initialising ]=====" << std::endl;
}   // end ctor


// private
FaceModelManager::~FaceModelManager()
{
    closeAll(); // Ensure all models closed
}   // end dtor


// public
std::string FaceModelManager::load( const std::string& fname)
{
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
    return err;
}   // end load


// public
std::string FaceModelManager::save( FaceModel* fmodel, const std::string& fname)
{
    std::string err;
    if ( !isValidExportFilename( fname))
        err = fname + " has unsupported extension!";
    else
    {
        const std::string fext = FaceTools::getExtension(fname);
        FileIOInterface* fileio = _fileInterfaces.at( fext);
        assert( fileio->canExport());
        fileio->setFileOp( fname, fmodel);
        fileio->process();
    }   // end else
    return err;
}   // end save


// private slot
void FaceModelManager::postProcessIO( FaceModel* fmodel, const std::string& errmsg)
{
    const std::string filename = qobject_cast<FileIOInterface*>( sender())->getFilepath();

    if ( !fmodel)   // Either a save or fail
    {
        if ( errmsg.empty())
            std::cerr << "Saved " << filename << std::endl;
        else
            std::cerr << "[ERROR] FaceTools::FaceModelManager::postProcessIO: " << errmsg << std::endl;
        emit finishedIO( fmodel, errmsg);
        return;
    }   // end if

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
        emit finishedIO( fmodel, oss.str());
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
            emit finishedIO( NULL, "Failed to make model into a triangulated manifold!");
            return;
        }   // end if
    }   // end if

    fmodel->updateMesh(model);
    emit finishedIO( fmodel, errmsg);
}   // end postProcessIO


// public
bool FaceModelManager::close( const FaceModel* fmodel)
{
    if ( _fmodels.count(fmodel) == 0)
        return false;
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
void FaceModelManager::setPluginsLoader( QTools::PluginsLoader *ploader)
{
    connect( ploader, SIGNAL( onLoadedPlugin( QTools::PluginInterface*)),
                this, SLOT( doOnLoadedPlugin( QTools::PluginInterface*)));
}   // end setPluginsLoader


// private slot
void FaceModelManager::doOnLoadedPlugin( QTools::PluginInterface *plugin)
{
    FileIOInterface* fileio = qobject_cast<FileIOInterface*>(plugin);
    if ( fileio)
    {
        populatePlugins( fileio);
        _importFilters = createFilters( _importExtDescMap);
        _exportFilters = createFilters( _exportExtDescMap);
    }   // end if
}   // end doOnLoadedPlugin


// private
void FaceModelManager::populatePlugins( FaceTools::FileIOInterface* fileios)
{
    const QStringList ids = fileios->getInterfaceIds();
    foreach ( const QString& id, ids)
    {
        FileIOInterface* fileio = qobject_cast<FileIOInterface*>( fileios->getInterface( id));
        if ( !fileio)
        {
            std::cerr << "[ERROR] FaceModelManager::populatePlugins: PluginInterface is not a FileIOInterface!" << std::endl;
            continue;
        }    // end if

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

        connect( fileio, SIGNAL( finished( FaceModel*, const std::string&)),
                   this, SLOT( postProcessIO( FaceModel*, const std::String&)));
    }   // end foreach
}   // end populatePlugins

