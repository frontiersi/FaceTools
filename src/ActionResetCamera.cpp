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

#include <ActionResetCamera.h>
using FaceTools::ActionResetCamera;
using FaceTools::ModelInteractor;
using FaceTools::FaceView;


ActionResetCamera::ActionResetCamera( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon(fname.c_str()),
      _fview(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionResetCamera::addInteractor( ModelInteractor* interactor)
{
    _fview = interactor->getView();
    checkEnable();
    if ( isEnabled())
        doAction();
}   // end addInteractor


void ActionResetCamera::setInteractive( ModelInteractor* interactor, bool enable)
{
    _fview = NULL;
    if ( enable)
        _fview = interactor->getView();
    checkEnable();
}   // end setInteractive


bool ActionResetCamera::doAction()
{
    _fview->setCameraToOrigin();
    return true;
}   // end doAction


void ActionResetCamera::checkEnable()
{
    setEnabled(_fview);
}   // end checkEnable


