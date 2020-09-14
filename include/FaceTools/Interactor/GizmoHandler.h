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

#ifndef FACE_TOOLS_GIZMO_HANDLER_H
#define FACE_TOOLS_GIZMO_HANDLER_H

#include <FaceTools/Vis/FaceView.h>

namespace FaceTools { namespace Interactor {

class MouseHandler;

class FaceTools_EXPORT GizmoHandler : public QObject
{ Q_OBJECT  // Inherits QObject so child classes don't need to multiply inherit
public:
    virtual void postRegister(){}
    virtual void refresh(){}

    void setEnabled( bool v) { _enabled = v;}
    bool isEnabled() const { return _enabled;}

protected:
    GizmoHandler();
    virtual ~GizmoHandler(){}

    virtual bool doEnterViewer(){ return false;}

    virtual bool doEnterModel(){ return false;}
    virtual bool doLeaveModel(){ return false;}

    virtual bool doEnterProp(){ return false;}
    virtual bool doLeaveProp(){ return false;}

    virtual bool doLeftButtonDown(){ return false;}
    virtual bool doLeftButtonUp(){ return false;}
    virtual bool doLeftDoubleClick(){ return false;}
    virtual bool doLeftDrag(){ return false;}

    virtual bool doMiddleButtonDown(){ return false;}
    virtual bool doMiddleButtonUp(){ return false;}
    virtual bool doMiddleDrag(){ return false;}

    virtual bool doRightButtonDown(){ return false;}
    virtual bool doRightButtonUp(){ return false;}
    virtual bool doRightDoubleClick(){ return false;}
    virtual bool doRightDrag(){ return false;}

    virtual bool doMouseMove(){ return false;}
    virtual bool doMouseWheelForward(){ return false;}
    virtual bool doMouseWheelBackward(){ return false;}

    inline FMV* viewer() const { return _vwr;}
    inline const vtkProp* prop() const { return _prop;}

private:
    bool _enabled;
    FMV *_vwr;
    const vtkProp *_prop;

    inline void _setViewer( FMV *fmv) { _vwr = fmv;}
    inline void _setProp( const vtkProp *p) { _prop = p;}

    friend class MouseHandler;

    GizmoHandler( const GizmoHandler&) = delete;
    void operator=( const GizmoHandler&) = delete;
};  // end class

}}   // end namespaces

#endif
