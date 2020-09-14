/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Interactor/ActionClickHandler.h>
#include <Action/ModelSelector.h>
#include <FaceModelViewer.h>
using FaceTools::Interactor::ActionClickHandler;
using FaceTools::Action::FaceAction;
using MS = FaceTools::Action::ModelSelector;


ActionClickHandler::Ptr ActionClickHandler::create() { return Ptr( new ActionClickHandler);}


void ActionClickHandler::refresh()
{
    setEnabled( MS::selectedView());
}   // end refresh


void ActionClickHandler::addLeftDoubleClickAction( FaceAction* a) { _actions.push_back(a);}


bool ActionClickHandler::doLeftDoubleClick()
{
    // Prime actions and return first ready
    FaceAction *act = nullptr;
    const QPoint mpos = MS::selectedViewer()->mouseCoords();
    for ( FaceAction *a : _actions)
    {
        if (a->primeMousePos( mpos))
        {
            act = a;
            break;
        }   // end if
    }   // end for

    if ( act)
        act->execute( Action::Event::USER);

    return act != nullptr;
}   // end doLeftDoubleClick
