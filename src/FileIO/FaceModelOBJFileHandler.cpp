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

#include <FaceModelOBJFileHandler.h>
#include <AssetImporter.h>  // RModelIO
#include <iomanip>
using FaceTools::FileIO::FaceModelOBJFileHandler;
using FaceTools::FaceModel;


FaceModelOBJFileHandler::FaceModelOBJFileHandler()
    : _importer( true,true)
{
    _importer.enableFormat("obj");
    for ( const std::string& ext : _exporter.getExtensions())
        _exts.insert(ext.c_str());
}   // end ctor


FaceModel* FaceModelOBJFileHandler::read( const QString& fname)
{
    _err = "";
    FaceModel* fm = NULL;
    const RFeatures::ObjModel::Ptr model = _importer.load( fname.toStdString());
    if ( model)
    {
        fm = new FaceModel;
        fm->model() = model;
    }   // end if
    else
        _err = ("Failed to load object from \"" + fname.toStdString() + "\"").c_str();
    return fm;
}   // end read


bool FaceModelOBJFileHandler::write( const FaceModel* fm, const QString& qfname)
{
    _err = "";
    const std::string fname = qfname.toStdString();
    const RFeatures::ObjModel::Ptr model = fm->model();
    std::cerr << "[STATUS] FaceTools::FileIO::FaceModelOBJFileHandler::write: Saving model to \"" << fname << "\"" << std::endl;
    if ( _exporter.save( model, fname))
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelOBJFileHandler::write: Saved model to \"" << fname << "\"" << std::endl;
    else
    {
        _err = _exporter.err().c_str();
        std::cerr << "[ERROR] FaceTools::FileIO::FaceModelOBJFileHandler::write: " << _err.toStdString() << std::endl;
    }   // end else
    return _err.isEmpty();
}   // end save


QString FaceModelOBJFileHandler::getFileDescription() const
{
    return _exporter.getDescription( _exporter.getExtensions()[0]).c_str();
}   // end getFileDescription

