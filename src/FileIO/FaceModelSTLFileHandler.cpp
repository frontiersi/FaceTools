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

#include <FileIO/FaceModelSTLFileHandler.h>
#include <QDebug>
using FaceTools::FileIO::FaceModelSTLFileHandler;
using FaceTools::FM;


FaceModelSTLFileHandler::FaceModelSTLFileHandler() : _importer( true,true)
{
    _importer.enableFormat("stl");
    _exporter.enableFormat("stl");
    for ( const std::string& ext : _exporter.getExtensions())
        _exts.insert(ext.c_str());
}   // end ctor


FM* FaceModelSTLFileHandler::read( const QString& qfname)
{
    _err = "";
    FM* fm = nullptr;
    r3d::Mesh::Ptr model = _importer.load(qfname.toStdString());
    if ( model)
        fm = new FM(model);
    else
        _err = _importer.err().c_str();
    return fm;
}   // end read


bool FaceModelSTLFileHandler::write( const FM* fm, const QString& qfname)
{
    _err = "";
    const std::string fname = qfname.toStdString();
    const r3d::Mesh& model = fm->mesh();
    std::cerr << QString("Saving to '%1'").arg(qfname).toStdString() << std::endl;
    if ( !_exporter.save( model, fname))
    {
        _err = _exporter.err().c_str();
        qWarning() << _err;
    }   // end else
    return _err.isEmpty();
}   // end write


QString FaceModelSTLFileHandler::getFileDescription() const
{
    return _exporter.getDescription( _exporter.getExtensions()[0]).c_str();
}   // end getFileDescription

