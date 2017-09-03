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

#ifndef FACE_TOOLS_MODEL_VIEWER_VTK_INTERFACE_H
#define FACE_TOOLS_MODEL_VIEWER_VTK_INTERFACE_H

#include "ModelViewerQtInterface.h"
#include <vtkInteractorStyleTrackballCamera.h>

namespace FaceTools
{

class FaceTools_EXPORT ModelViewerVTKInterface : public vtkInteractorStyleTrackballCamera
{
public:
    static ModelViewerVTKInterface* New();
    vtkTypeMacro( ModelViewerVTKInterface, vtkInteractorStyleTrackballCamera)

    ModelViewerQtInterface* getQtInterface() { return &_qtinterface;}

    // Disable normal VTK key press operations (so that Qt event handling takes over)
    virtual void OnChar() {}
    virtual void OnKeyDown() {}
    virtual void OnKeyPress() {}

    virtual void OnLeftButtonDown();
    virtual void OnLeftButtonUp();

    virtual void OnRightButtonDown();
    virtual void OnRightButtonUp();

    virtual void OnMiddleButtonDown();
    virtual void OnMiddleButtonUp();

    virtual void OnMouseWheelForward();
    virtual void OnMouseWheelBackward();
    
    virtual void OnMouseMove();

    virtual void OnEnter();
    virtual void OnLeave();

protected:
    ModelViewerVTKInterface();
    virtual ~ModelViewerVTKInterface(){}

private:
    ModelViewerQtInterface _qtinterface;
    QPoint getMouseCoords(); // With top left origin

    ModelViewerVTKInterface( const ModelViewerVTKInterface&);  // NO COPY
    void operator=( const ModelViewerVTKInterface&);           // NO COPY
};  // end class

}   // end namespace

#endif

