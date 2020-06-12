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


int FaceActionWorker::_s_userWorkCount(0);
QMutex FaceActionWorker::_s_mutex;


FaceActionWorker::FaceActionWorker( FaceAction* worker, Event e)
    : QThread(worker), _worker(worker), _event(e), _timer(nullptr), _tcount(0)
{
    if ( e == Event::USER)
    {
        _s_mutex.lock();
        _s_userWorkCount++;
        _s_mutex.unlock();

        MS::setCursor( Qt::CursorShape::BusyCursor);
        _tcount = 0;
        _timer = new QTimer(this);
        _timer->setTimerType( Qt::CoarseTimer);
        _status = MS::currentStatus();
        connect( _timer, &QTimer::timeout, this, &FaceActionWorker::_doOnTimerInterval);
        _timer->start( 1000);   // Once per second
    }   // end if
}   // end ctor


FaceActionWorker::~FaceActionWorker()
{
    _deleteTimer();

    if ( _event == Event::USER)
    {
        _s_mutex.lock();
        _s_userWorkCount--;
        assert( _s_userWorkCount >= 0);
        _s_mutex.unlock();

        MS::restoreCursor();
        MS::clearStatus();
    }   // end if
}   // end dtor


void FaceActionWorker::run()    // thread function
{
    _worker->doAction( _event);
    emit onWorkFinished( _event);
}   // end run


void FaceActionWorker::_doOnTimerInterval()
{
    static const int MAX_TIME_OUT = 60;  // seconds
    _tcount++;
    if ( _tcount == MAX_TIME_OUT)
    {
        requestInterruption();
//        _status += " timed out!";
        _deleteTimer();
        _worker->endNow();
    }   // end if
    else if ( _tcount < MAX_TIME_OUT)
        _status += ".";
    MS::showStatus( _status);
}   // end _doOnTimerInterval


void FaceActionWorker::_deleteTimer()
{
    if ( _timer)
    {
        delete _timer;
        _timer = nullptr;
    }   // end if
}   // _deleteTimer
