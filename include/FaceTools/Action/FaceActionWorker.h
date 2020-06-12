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

#ifndef FACE_TOOLS_ACTION_FACE_ACTION_WORKER_H
#define FACE_TOOLS_ACTION_FACE_ACTION_WORKER_H

#include <FaceTools/FaceTypes.h>
#include <QTimer>
#include <QThread>
#include <QMutex>

namespace FaceTools { namespace Action {

class FaceAction;

class FaceTools_EXPORT FaceActionWorker : public QThread
{ Q_OBJECT
public:
    FaceActionWorker( FaceAction*, Event);
    virtual ~FaceActionWorker();

    // True if at least one user instigated worker is active
    static bool isUserWorking() { return _s_userWorkCount > 0;}

signals:
    void onWorkFinished( Event);

protected:
    void run() override;

private slots:
    void _doOnTimerInterval();

private:
    FaceAction* _worker;
    Event _event;
    QTimer *_timer;
    int _tcount;
    QString _status;

    static int _s_userWorkCount;
    static QMutex _s_mutex;

    void _deleteTimer();
};  // end class

}}   // end namespace

#endif
