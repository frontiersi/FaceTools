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

/**
 * Interactive functionality for the ModelViewer.
 * Ties VTK's event system to Qt's.
 */

#ifndef FACE_TOOLS_MODEL_VIEWER_QT_INTERFACE_H
#define FACE_TOOLS_MODEL_VIEWER_QT_INTERFACE_H

#include "FaceTools_Export.h"
#include <QObject>
#include <QPoint>

namespace FaceTools
{

class ModelViewerVTKInterface;

class FaceTools_EXPORT ModelViewerQtInterface : public QObject
{ Q_OBJECT
public:
    ModelViewerQtInterface();

    // Return the most recent mouse coords having top left origin.
    const QPoint& getMouseCoords() const { return _mcoords;}

    // Camera is only unlocked if the user left double clicks.
    bool isCameraLocked() const { return _cameraLocked;}

signals:
    // Clients should listen to these signals to know what user input events from VTK are being forwarded.
    void onMouseLeave();
    void onMouseEnter();
    void onMouseMove();
    void onMouseWheelForward();
    void onMouseWheelBackward();
    void onMiddleButtonDown();
    void onMiddleButtonUp();
    void onRightButtonDown();
    void onRightButtonUp();
    void onLeftButtonDown();  // Not emitted on a double click left down
    void onLeftButtonUp();
    void onLeftDoubleClick();

    void lockedCamera();      // Locked camera from panning, rotating, and zooming
    void unlockedCamera();    // Unlocked camera from panning, rotating, and zooming

private:
    bool _cameraLocked;
    qint64 _lbDownTime; // If not 0, last time left mouse button went down
    QPoint _mcoords;    // Last set mouse coords

    // No copying
    ModelViewerQtInterface( const ModelViewerQtInterface&);
    void operator=( const ModelViewerQtInterface&);

    // The interface that VTK uses to tell Qt clients what user inputs to respond to.
    friend class ModelViewerVTKInterface;
    // Calling these functions causes the corresponding signals to be emitted from this object.
    // The current mouse X,Y position should always be provided.
    void signalOnMouseLeave( const QPoint&);
    void signalOnMouseEnter( const QPoint&);
    void signalOnMouseMove( const QPoint&);
    void signalOnMouseWheelForward( const QPoint&);
    void signalOnMouseWheelBackward( const QPoint&);
    void signalOnMiddleButtonDown( const QPoint&);
    void signalOnMiddleButtonUp( const QPoint&);
    void signalOnRightButtonDown( const QPoint&);
    void signalOnRightButtonUp( const QPoint&);
    void signalOnLeftButtonDown( const QPoint&);
    void signalOnLeftButtonUp( const QPoint&);
};  // end class

}   // end namespace

#endif


