/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <FileIO/BackgroundModelLoader.h>
using FaceTools::FileIO::BackgroundModelLoader;
using RFeatures::ObjModel;

BackgroundModelLoader::BackgroundModelLoader( const std::string& fpath) : _fpath(fpath) {}

void BackgroundModelLoader::run()
{
    ObjModel::Ptr model;
    if ( FMM::isOpen(_fpath))
    {
        // If the model is already loaded, then use a snapshot of it right now.
        FM* fm = FMM::model(_fpath);
        model = fm->model().deepCopy(false);
    }   // end if
    else
    {
        // Otherwise, load it in and take the model.
        FM* fm = FMM::read( _fpath);
        model = fm->wmodel();
        FMM::close(fm);
    }   // end else

    emit loadedModel( model);
}   // end run
