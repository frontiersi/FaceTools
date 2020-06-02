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

#include <FileIO/AsyncModelLoader.h>
using FaceTools::FileIO::AsyncModelLoader;

AsyncModelLoader::AsyncModelLoader( const std::string& fpath) : _fpath(fpath) {}

void AsyncModelLoader::run()
{
    FM *fm = nullptr;
    if ( FMM::isOpen(_fpath))
        fm = FMM::model(_fpath);
    else
        fm = FMM::read( _fpath); // Otherwise, load it in and take the model.
    emit loadedModel( fm);
}   // end run
