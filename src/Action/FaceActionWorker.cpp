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

#include <Action/FaceActionWorker.h>
#include <Action/FaceAction.h>
#include <Action/ModelSelector.h>
using FaceTools::Action::FaceActionWorker;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


size_t FaceActionWorker::_workerCount(0);


FaceActionWorker::FaceActionWorker( FaceAction* worker, Event e)
    : QThread(worker), _worker(worker), _event(e), _timer(nullptr)
{
    if ( e == Event::USER)
    {
        _workerCount++;
        if ( _workerCount == 1)
        {
            MS::setCursor( Qt::CursorShape::BusyCursor);
            _timer = new QTimer(this);
            _timer->setTimerType( Qt::VeryCoarseTimer);
            connect( _timer, &QTimer::timeout, [](){ MS::showStatus( MS::currentStatus() + ".");});
            _timer->start( 1000);   // Once per second
        }   // end if
    }   // end if
}   // end ctor


FaceActionWorker::~FaceActionWorker()
{
    if ( _event == Event::USER)
    {
        _workerCount--;
        if ( _workerCount == 0)
            MS::restoreCursor();
    }   // end if

    if ( _timer)
        delete _timer;
    _timer = nullptr;
}   // end dtor


void FaceActionWorker::run()
{
    _worker->doAction( _event);
    emit workerFinished( _event);
}   // end run
