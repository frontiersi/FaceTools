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

#include <CloseFaceModelsHelper.h>
#include <algorithm>
using FaceTools::FileIO::CloseFaceModelsHelper;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceModel;


CloseFaceModelsHelper::CloseFaceModelsHelper( FaceModelManager* fmm, QWidget *parent)
    : _fmm(fmm), _parent(parent)
{}


void CloseFaceModelsHelper::close( FaceModel* fm)
{
    emit onClose(fm);
    _fmm->close(fm);
}   // end close


void CloseFaceModelsHelper::close( const std::unordered_set<FaceModel*>& fset)
{
    std::for_each( std::begin(fset), std::end(fset), [this](auto fm){ this->close(fm);});
}   // end close
