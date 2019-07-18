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

#ifndef FACE_TOOLS_FACE_VIEW_INTERACTOR_H
#define FACE_TOOLS_FACE_VIEW_INTERACTOR_H

/**
 * Deals with messages relating to position of the mouse cursor as it moves over a face model.
 */

#include "MouseHandler.h"
#include <FaceView.h>
#include <vtkProp.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT FaceViewInteractor : public MouseHandler
{ Q_OBJECT
public:
    FaceViewInteractor();

    Vis::FV* view() const { return _mnow;}          // FaceView cursor is currently over (null if none).
    const vtkProp* prop() const { return _pnow;}    // Prop cursor is over (null if none).

protected:
    virtual void enterModel( Vis::FV*) {}
    virtual void leaveModel( Vis::FV*) {}

    // Enter and leave signals for props on their associated views.
    // Can only work if the props are pickable.
    virtual void enterProp( Vis::FV*, const vtkProp*) {}
    virtual void leaveProp( Vis::FV*, const vtkProp*) {}

    void enterViewer( FMV*) override;
    void leaveViewer( FMV*) override;

    bool leftButtonDown() override;
    bool leftDoubleClick() override;
    bool mouseMove() override;
    bool leftDrag() override;
    bool middleDrag() override;
    bool rightDrag() override;

private:
    Vis::FV* _mnow;
    const vtkProp* _pnow;
    bool testPoint();
};  // end class

}}   // end namespace

#endif
