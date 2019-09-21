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

#ifndef FACE_TOOLS_MOUSE_HANDLER_H
#define FACE_TOOLS_MOUSE_HANDLER_H

#include <FaceTypes.h>
#include <VtkMouseHandler.h>
#include <vtkRenderWindow.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT MouseHandler : public QTools::VMH
{
public:
    MouseHandler();
    ~MouseHandler() override;

    // Return the viewer the mouse pointer was last over (never returns null).
    FMV* mouseViewer() const { return _vwr;}

protected:
    // Called when the mouse has entered the parameter viewer (mouseViewer() returns parameter).
    virtual void enterViewer( FMV*) {}
    // Called when the mouse has left the parameter viewer.
    virtual void leaveViewer( FMV*) {}

/*
    // Override any of the following virtual functions to implement
    // new behaviour on mouse input events. Returning true from any
    // of these functions will prevent the input from being used in
    // the standard camera/actor movement interactions.
    virtual bool mouseMove( ){ return false;}  // Move mouse with no buttons depressed.

    virtual bool leftButtonDown( ){ return false;}  // Not called if leftDoubleClick
    virtual bool leftButtonUp( ){ return false;}    // Not called if leftDoubleClick
    virtual bool leftDoubleClick( ){ return false;}

    virtual bool middleButtonDown( ){ return false;}
    virtual bool middleButtonUp( ){ return false;}

    virtual bool rightButtonDown( ){ return false;}
    virtual bool rightButtonUp( ){ return false;}

    virtual bool leftDrag( ){ return false;}   // Move mouse with left button depressed.
    virtual bool rightDrag( ){ return false;}  // Move mouse with right button depressed.
    virtual bool middleDrag( ){ return false;} // Move mouse with middle button depressed.

    virtual bool mouseWheelForward( ){ return false;}
    virtual bool mouseWheelBackward( ){ return false;}
*/

private:
    void mouseEnter( const QTools::VtkActorViewer*) override;
    void mouseLeave( const QTools::VtkActorViewer*) override;
    std::unordered_map<const vtkRenderWindow*, FMV*> _vwrs;
    FMV* _vwr;

    MouseHandler( const MouseHandler&) = delete;
    void operator=( const MouseHandler&) = delete;
};  // end class

}}   // end namespace

#endif
