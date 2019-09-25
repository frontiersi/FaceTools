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

#ifndef FACE_TOOLS_PROP_NOTIFIER_H
#define FACE_TOOLS_PROP_NOTIFIER_H

#include "MouseHandler.h"
#include "../Vis/FaceView.h"
#include <vtkProp.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT PropNotifier : public QObject, public MouseHandler
{ Q_OBJECT
public:
    PropNotifier();

    Vis::FV* view() const { return _mnow;}          // FaceView cursor is currently over (null if none).
    const vtkProp* prop() const { return _pnow;}    // Prop cursor is over (null if none).

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

    bool _testPoint();
    Vis::FV* _mnow;
    const vtkProp* _pnow;
};  // end class

}}   // end namespace

#endif
