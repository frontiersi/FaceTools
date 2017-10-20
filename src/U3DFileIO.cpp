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

#include <U3DFileIO.h>
#include <FaceModel.h>
using FaceTools::U3DFileIO;
using FaceTools::FaceModel;


U3DFileIO::U3DFileIO()
    : FaceTools::FileIOInterface(),
      _exporter( new RModelIO::U3DExporter( true/* delete transition files*/))
{
    init();
}   // end ctor


U3DFileIO::~U3DFileIO()
{
    delete _exporter;
}   // end dtor


QString U3DFileIO::getDisplayName() const
{
    if ( !_exporter->isSupported())
        return "";
    const std::vector<std::string>& exts = _exporter->getExtensions();
    return _exporter->getDescription( exts[0]).c_str();
}   // end getDisplayName


size_t U3DFileIO::getFileExtensions( std::vector<std::string>& exts) const
{
    const std::vector<std::string>& texts = _exporter->getExtensions();   // May be empty
    exts.insert(exts.end(), texts.begin(), texts.end());
    return texts.size();
}   // end getFileExtensions


bool U3DFileIO::canExport() const
{
    return _exporter->isSupported();
}   // end canExport


bool U3DFileIO::save( const FaceModel* fmodel, const std::string& fname)
{
    _err = "";
    if ( !_exporter->save( fmodel->getObjectMeta()->getObject(), fname))
    {
        _err = _exporter->err();
        return false;
    }   // end if
    return true;
}   // end save


