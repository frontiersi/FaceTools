/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <FileIO/FaceModel3DSFileHandler.h>
#include <QDebug>
using FaceTools::FileIO::FaceModel3DSFileHandler;
using FaceTools::FM;


FaceModel3DSFileHandler::FaceModel3DSFileHandler() : _importer( true,true)
{
    _importer.enableFormat("3ds");
    _exporter.enableFormat("3ds");
    for ( const std::string& ext : _exporter.getExtensions())
        _exts.insert(ext.c_str());
}   // end ctor


FM* FaceModel3DSFileHandler::read( const QString& qfname)
{
    _err = "";
    FM* fm = nullptr;
    r3d::Mesh::Ptr model = _importer.load(qfname.toLocal8Bit().toStdString());
    if ( model)
        fm = new FM(model);
    else
        _err = _importer.err().c_str();
    return fm;
}   // end read


bool FaceModel3DSFileHandler::write( const FM* fm, const QString& qfname)
{
    _err = "";
    const std::string fname = qfname.toLocal8Bit().toStdString();
    const r3d::Mesh& model = fm->mesh();
    if ( !_exporter.save( model, fname))
    {
        _err = _exporter.err().c_str();
        qWarning() << _err;
    }   // end else
    return _err.isEmpty();
}   // end write


QString FaceModel3DSFileHandler::getFileDescription() const
{
    return _exporter.getDescription( _exporter.getExtensions()[0]).c_str();
}   // end getFileDescription

