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

#ifndef FACE_TOOLS_PROP_NOTIFIER_H
#define FACE_TOOLS_PROP_NOTIFIER_H

#include "MouseHandler.h"
#include <FaceTools/Vis/FaceView.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT PropNotifier : public QObject, public MouseHandler
{ Q_OBJECT
public:
    PropNotifier();

    const vtkProp* prop() const { return _pnow;}    // Prop cursor is over (null if none).

    // Call to force a new test of what's under the mouse cursor.
    // This function is called whenever a mouse action is carried
    // out but can be called explicitly too. Do this when removing props
    // to ensure that signals reflecting the current state are emitted.
    void testMouse();

signals:
    void onEnterModel( Vis::FV*);
    void onLeaveModel( Vis::FV*);

    // Enter and leave signals for props on their associated views. Only works if the props are pickable.
    void onEnterProp( Vis::FV*, const vtkProp*);
    void onLeaveProp( Vis::FV*, const vtkProp*);

protected:
    virtual void enterViewer( FMV*) {}
    virtual void leaveViewer( FMV*) {}

private:
    bool mouseMove() override;
    bool leftDrag() override;
    bool middleDrag() override;
    bool rightDrag() override;

    Vis::FV* _mnow;
    const vtkProp* _pnow;
};  // end class

}}   // end namespace

#endif
