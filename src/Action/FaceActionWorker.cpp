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

#include <FaceActionWorker.h>
#include <FaceAction.h>
using FaceTools::Action::FaceActionWorker;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;

FaceActionWorker::FaceActionWorker( FaceAction* worker, FaceControlSet* rset, const QPoint& tp)
    : QThread(worker), _worker(worker), _rset(rset), _tp(tp)
{
}   // end ctor

void FaceActionWorker::run()
{
    const bool rv = _worker->doAction( *_rset, _tp);
    emit workerFinished(rv);
}   // end run

