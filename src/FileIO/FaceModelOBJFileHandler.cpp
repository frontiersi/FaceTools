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

#include <FaceModelOBJFileHandler.h>
#include <AssetImporter.h>  // RModelIO
#include <QDebug>
#include <iomanip>
using FaceTools::FileIO::FaceModelOBJFileHandler;
using FaceTools::FM;


FaceModelOBJFileHandler::FaceModelOBJFileHandler()
    : _importer( true,true)
{
    _importer.enableFormat("obj");
    for ( const std::string& ext : _exporter.getExtensions())
        _exts.insert(ext.c_str());
}   // end ctor


FM* FaceModelOBJFileHandler::read( const QString& fname)
{
    _err = "";
    FM* fm = nullptr;
    RFeatures::ObjModel::Ptr model = _importer.load( fname.toStdString());
    if ( model)
    {
        RFeatures::ObjModelInfo::Ptr minfo = RFeatures::ObjModelInfo::create(model);
        if ( minfo)
            fm = new FM(minfo);
        else
            _err = QString("Failed to clean '%1'").arg( fname);
    }   // end if
    else
        _err = QString("Failed to load '%1'").arg(fname);
    return fm;
}   // end read


bool FaceModelOBJFileHandler::write( const FM* fm, const QString& qfname)
{
    _err = "";
    const std::string fname = qfname.toStdString();
    const RFeatures::ObjModel* model = fm->info()->cmodel();
    qInfo() << QString("Saving to '%1'").arg(qfname);
    if ( !_exporter.save( model, fname))
    {
        _err = _exporter.err().c_str();
        qWarning() << _err;
    }   // end else
    return _err.isEmpty();
}   // end save


QString FaceModelOBJFileHandler::getFileDescription() const
{
    return _exporter.getDescription( _exporter.getExtensions()[0]).c_str();
}   // end getFileDescription

