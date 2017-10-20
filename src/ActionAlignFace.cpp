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

#include <ActionAlignFace.h>
#include <FaceModel.h>
using FaceTools::ActionAlignFace;
using FaceTools::ModelInteractor;
using FaceTools::FaceModel;


ActionAlignFace::ActionAlignFace( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _fmodel(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionAlignFace::setInteractive( ModelInteractor* interactor, bool enable)
{
    _fmodel = NULL;
    if ( !enable)
        interactor->getModel()->disconnect( this);
    else
    {
        _fmodel = interactor->getModel();
        connect( _fmodel, &FaceModel::onFaceDetected, this, &ActionAlignFace::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractive


bool ActionAlignFace::doAction()
{
    assert(_fmodel);
    _fmodel->transformToStandardPosition();
    return true;
}   // end doAction


void ActionAlignFace::checkEnable()
{
    setEnabled( _fmodel && _fmodel->isDetected());
}   // end checkEnable


