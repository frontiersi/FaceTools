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

#ifndef FACE_TOOLS_INTERACTION_INTERFACE_H
#define FACE_TOOLS_INTERACTION_INTERFACE_H

#include "ModelViewerQtInterface.h"
#include <QAction>

namespace FaceTools
{

class FaceTools_EXPORT InteractionInterface : public QObject
{ Q_OBJECT
public:
    InteractionInterface();

    // Set with NULL to disconnect.
    void setInterface( ModelViewerQtInterface*);

    // Get the latest mouse coordinates. It's an error to
    // call this function before calling setInterface.
    const QPoint& getMouseCoords() const;

    bool isCameraLocked() const;
    void setCameraLocked( bool); // Lock/unlock camera movement
   
signals:
    void mousePressed( const QPoint&); // Generic mouse button press event

protected:
    virtual void mouseMove( const QPoint&){}

    virtual void middleButtonDown( const QPoint&){}
    virtual void middleButtonUp( const QPoint&){}
    virtual void middleDrag( const QPoint&){}

    virtual void rightButtonDown( const QPoint&){}
    virtual void rightButtonUp( const QPoint&){}
    virtual void rightDrag( const QPoint&){}

    virtual void leftButtonDown( const QPoint&){}
    virtual void leftButtonUp( const QPoint&){}
    virtual void leftDoubleClick( const QPoint&){}
    virtual void leftDrag( const QPoint&){}

private slots:
    void doOnMiddleButtonDown();
    void doOnMiddleButtonUp();
    void doOnRightButtonDown();
    void doOnRightButtonUp();
    void doOnLeftButtonDown();
    void doOnLeftButtonUp();
    void doOnLeftDoubleClick();
    void doOnMouseMove();

private:
    bool _rbdown;
    bool _lbdown;
    bool _mbdown;
    ModelViewerQtInterface* _qinterface;
    InteractionInterface( const InteractionInterface&); // No copy
    void operator=( const InteractionInterface&);       // No copy
};  // end class
                                            
}   // end namespace

#endif
