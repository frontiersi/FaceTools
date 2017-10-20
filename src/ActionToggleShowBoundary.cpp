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

#include <ActionToggleShowBoundary.h>
using FaceTools::ActionToggleShowBoundary;
using FaceTools::ModelInteractor;


ActionToggleShowBoundary::ActionToggleShowBoundary( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _interactor(NULL)
{
    init();
    setCheckable(true, false);
    checkEnable();
}   // end ctor


void ActionToggleShowBoundary::setInteractive( ModelInteractor* interactor, bool enable)
{
    _interactor = NULL;
    if ( !enable)
        interactor->getModel()->disconnect( this);
    else
    {
        _interactor = interactor;
        connect( _interactor->getModel(), &FaceTools::FaceModel::onFaceDetected, this, &ActionToggleShowBoundary::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractive



bool ActionToggleShowBoundary::doAction()
{
    assert( _interactor);
    _interactor->getView()->showBoundary( isChecked());
    return true;
}   // end doAction


// private slot
void ActionToggleShowBoundary::checkEnable()
{
    setEnabled( _interactor && _interactor->getModel()->isDetected());
}   // end checkEnable


