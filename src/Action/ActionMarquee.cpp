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

#include <Action/ActionMarquee.h>
#include <FaceModelViewer.h>
#include <algorithm>
#include <functional>
using FaceTools::Action::ActionMarquee;
using FaceTools::Action::CameraWorker;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FaceModelViewer;
using CW = FaceTools::Action::CameraWorker;
using MS = FaceTools::Action::ModelSelector;


ActionMarquee::ActionMarquee( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable(true, false);
    for ( FMV* fmv : MS::viewers())
        _workers.push_back( new CW( fmv));
    addRefreshEvent( Event::CAMERA_CHANGE);
}   // end ctor


ActionMarquee::~ActionMarquee()
{
    std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){delete cw;});
}   // end dtor


bool ActionMarquee::update( Event e)
{
    bool chk = isChecked();
    if ( chk && has(e, Event::CAMERA_CHANGE))
    {
        _stopCameras();
        chk = false;
    }   // end if
    return chk;
}   // end update


bool ActionMarquee::isAllowed( Event e) { return MS::isViewSelected();}


void ActionMarquee::doAction( Event e)
{
    if ( isChecked())
        _startCameras();
    else
        _stopCameras();
}   // end doAction

Event ActionMarquee::doAfterAction( Event) { return Event::CAMERA_CHANGE;}


void ActionMarquee::_startCameras() { std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){cw->start();});}

void ActionMarquee::_stopCameras() { std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){cw->stop();});}
