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

#include <ActionSetFocus.h>
#include <FaceView.h>
using FaceTools::ActionSetFocus;
using FaceTools::ModelInteractor;
using FaceTools::FaceView;


ActionSetFocus::ActionSetFocus()
    : FaceTools::FaceAction(), _interactor(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionSetFocus::setInteractive( ModelInteractor* interactor, bool enable)
{
    _interactor = NULL;
    if ( !enable)
        interactor->disconnect( this);
    else
    {
        _interactor = interactor;
        connect( _interactor, &ModelInteractor::onEnteringLandmark, this, &ActionSetFocus::checkEnable);
        connect( _interactor, &ModelInteractor::onExitingLandmark, this, &ActionSetFocus::checkEnable);
        connect( _interactor, &ModelInteractor::onEnteringModel, this, &ActionSetFocus::checkEnable);
        connect( _interactor, &ModelInteractor::onExitingModel, this, &ActionSetFocus::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractive


bool ActionSetFocus::doAction()
{
    return _interactor->getView()->setCameraFocus( _interactor->getMouseCoords());
}   // end doAction


void ActionSetFocus::checkEnable()
{
    if ( !_interactor)
        setEnabled(false);
    else
    {
        // Allow if p on the model but not on a landmark
        const QPoint& p = _interactor->getMouseCoords();
        const FaceView* fview = _interactor->getView();
        setEnabled( fview->isPointedAt(p) && fview->isLandmarkPointedAt(p).empty());
    }   // end else
}   // end checkEnable

