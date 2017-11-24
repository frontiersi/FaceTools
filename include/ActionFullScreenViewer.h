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

#ifndef FACE_TOOLS_ACTION_FULLSCREEN_VIEWER_H
#define FACE_TOOLS_ACTION_FULLSCREEN_VIEWER_H

#include "InteractiveModelViewer.h"
#include <string>
#include <QIcon>
#include <QAction>

namespace FaceTools {

class FaceTools_EXPORT ActionFullScreenViewer : public QObject
{ Q_OBJECT
public:
    ActionFullScreenViewer( InteractiveModelViewer*, const std::string& dname="&Full Screen");
    virtual ~ActionFullScreenViewer();

    QAction* qaction() { return &_action;}

public slots:
    void toggleFullScreen();    // Fired by action

private:
    InteractiveModelViewer* _viewer;
    class KeyPressHandler;        // Derives from QTools::KeyPressHandler
    KeyPressHandler* _kphandler;  // implements virtual bool handleKeyPress(QKeyEvent*)
    QAction _action;
    bool _isfull;
};  // end class

}   // end namespace

#endif

