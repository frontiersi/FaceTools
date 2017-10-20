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

#include <ActionSaveSnapshot.h>
using FaceTools::ActionSaveSnapshot;
using FaceTools::ModelInteractor;


ActionSaveSnapshot::ActionSaveSnapshot( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str())
{
    init();
    checkEnable();
}   // end ctor


void ActionSaveSnapshot::addInteractor( ModelInteractor* interactor)
{
    _interactors.insert(interactor);
    checkEnable();
}   // end addInteractor


void ActionSaveSnapshot::removeInteractor( ModelInteractor* interactor)
{
    _interactors.erase(interactor);
    checkEnable();
}   // end removeInteractor


bool ActionSaveSnapshot::doAction()
{
    ModelInteractor* interactor = *_interactors.begin();
    interactor->getView()->saveSnapshot();
    return true;
}   // end doAction


void ActionSaveSnapshot::checkEnable()
{
    // Enabled as long as one model present
    setEnabled( !_interactors.empty());
}   // end checkEnable

