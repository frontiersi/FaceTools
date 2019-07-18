/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionMarquee.h>
#include <FaceModelViewer.h>
#include <algorithm>
#include <functional>
using FaceTools::Action::ActionMarquee;
using FaceTools::Action::CameraWorker;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FaceModelViewer;
using FaceTools::FVS;
using CW = FaceTools::Action::CameraWorker;
using MS = FaceTools::Action::ModelSelector;


ActionMarquee::ActionMarquee( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable(true, false);
    addTriggerEvent( Event::CAMERA_CHANGE);
    addTriggerEvent( Event::ACTOR_MOVE);
    for ( FMV* fmv : ModelSelector::viewers())
        _workers.insert( new CameraWorker( fmv));
}   // end ctor


ActionMarquee::~ActionMarquee()
{
    std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){delete cw;});
}   // end dtor


bool ActionMarquee::checkState( Event e)
{
    return isTriggerEvent(e) ? false : isChecked();
}   // end checkChecked


void ActionMarquee::doAction( Event)
{
    if ( isChecked())
    {
        std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){cw->start();});
        emit onEvent( Event::CAMERA_CHANGE);
    }   // end if
    else
        std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){cw->stop();});
}   // end doAction
