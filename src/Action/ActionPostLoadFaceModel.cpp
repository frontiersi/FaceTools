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

#include <ActionPostLoadFaceModel.h>
#include <FaceModelViewer.h>
using FaceTools::Action::ActionPostLoadFaceModel;
using FaceTools::Action::ActionSelect;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionPostLoadFaceModel::ActionPostLoadFaceModel( ActionSelect* selector, LoadFaceModelsHelper* lhelper)
    : _selector(selector)
{
    setExternalSelect(false);
    connect( lhelper, &LoadFaceModelsHelper::loadedModel, this, &ActionPostLoadFaceModel::processOnLoad);
}   // end ctor


void ActionPostLoadFaceModel::processOnLoad( FaceModel* fm)
{
    FaceControl *fc = new FaceControl(fm);
    _selector->viewer()->attach(fc);
    _selector->addFaceControl(fc);
    process(fc);    // Process on self ensures that follow-on actions execute
}   // end processOnLoad
