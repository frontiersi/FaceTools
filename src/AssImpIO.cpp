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

#include <AssImpIO.h>
#include <FaceModel.h>
#include <boost/algorithm/string.hpp>
using FaceTools::AssImpIO;
using FaceTools::FaceModel;

// public
AssImpIO::AssImpIO( const std::string& ext)
    : FaceTools::FileIOInterface(),
      _ext( boost::algorithm::to_lower_copy(ext)),
      _importer( new RModelIO::AssetImporter( true/*load textures*/, true/*fail on non triangles*/))
{
    init();

    // Check if we can import using this file extension
    if ( !_importer->enableFormat(_ext))
    {
        delete _importer;
        _importer = NULL;
    }   // end if
}   // end ctor


// public
AssImpIO::~AssImpIO()
{
    if ( _importer)
        delete _importer;
    _importer = NULL;
}   // end dtor


// private
const boost::unordered_map<std::string, std::string>* AssImpIO::available() const
{
    const boost::unordered_map<std::string, std::string>* av = NULL;
    if ( _importer)
        av = &_importer->getAvailable();
    assert(av);
    return av;
}   // end getAvailable


// public
std::string AssImpIO::getFileDescription() const
{
    const boost::unordered_map<std::string, std::string>* av = available();
    if ( av->count(_ext) == 0)
    {
        std::cerr << "[ERROR] FaceTools::AssImpIO: No entry in available formats map for required extension " << _ext << std::endl;
        return "";
    }   // end if
    return av->at(_ext);
}   // end getFileDescription


// public
size_t AssImpIO::getFileExtensions( std::vector<std::string>& exts) const
{
    exts.push_back(_ext);
    return 1;
}   // end getFileExtensions


// protected
FaceModel* AssImpIO::load( const std::string& fname)
{
    if ( !canImport())
        return NULL;

    _err = "";
    RFeatures::ObjModel::Ptr model = _importer->load(fname);
    if ( !model)
    {
        _err = _importer->err();
        return NULL;
    }   // end if

    FaceModel* fmodel = new FaceModel( FaceTools::ObjMetaData::create(fname, model));
    return fmodel;
}   // end load

