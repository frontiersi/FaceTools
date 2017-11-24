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

#include <VisualisationAction.h>
#include <FaceControl.h>
#include <cassert>
using FaceTools::VisualisationAction;
using FaceTools::FaceControl;


VisualisationAction::VisualisationAction()
    : FaceAction()
{
    setEnabled(false);
}   // end ctor


// protected slot
bool VisualisationAction::doAction()
{
    foreach ( FaceControl* fcont, _fconts)
        fcont->setVisualisation(this);
    return true;
}   // end doAction


// public
void VisualisationAction::addController( FaceControl* fcont)
{
    assert(fcont);
    if ( this->isDefault(fcont))
        fcont->setVisualisation(this);
}   // end addController


// public
void VisualisationAction::removeController( FaceControl *fcont)
{
    _fconts.erase(fcont);
    fcont->disconnect(this);
}   // end removeController


// public
void VisualisationAction::setControlled( FaceControl* fcont, bool enable)
{
    removeController(fcont);
    if ( enable)
    {
        recheckCanVisualise(fcont);
        connect( fcont, &FaceControl::meshUpdated, [=](){ this->recheckCanVisualise(fcont);});
        connect( fcont, &FaceControl::metaUpdated, [=](){ this->recheckCanVisualise(fcont);});
    }   // end else if
    setEnabled( !_fconts.empty());
}   // end setControlled


// private
void VisualisationAction::recheckCanVisualise( FaceControl* fcont)
{
    _fconts.erase(fcont);
    if ( this->isAvailable(fcont))
        _fconts.insert(fcont);
    setEnabled( !_fconts.empty());
}   // end recheckCanVisualise
