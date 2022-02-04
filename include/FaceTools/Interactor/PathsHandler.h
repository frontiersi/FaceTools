/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_PATHS_HANDLER_H
#define FACE_TOOLS_PATHS_HANDLER_H

#include "GizmoHandler.h"
#include <FaceTools/Vis/PathSetVisualisation.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT PathsHandler : public GizmoHandler
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<PathsHandler>;
    static Ptr create();

    void refresh() override;

    Vis::PathSetVisualisation &visualisation() { return _vis;}
    const Vis::PathSetVisualisation &visualisation() const { return _vis;}

    // The path being hovered over (if any).
    inline Vis::PathView::Handle* hoverPath() const { return _handle;}

    void addPath( Vec3f&);

    // Returns true iff a path was in the middle of being dragged otherwise does nothing.
    bool endDragging();

    inline bool isDragging() const { return _dragging;}

    int leavePath();    // Returns id of path that was left (_handle->pathId())

signals:
    void onStartedDrag( int pid, int hid);
    void onFinishedDrag( int pid, int hid);
    void onEnterHandle( int pid, int hid);
    void onLeaveHandle( int pid, int hid);

private:
    bool doEnterProp() override;
    bool doLeaveProp() override;
    bool doLeftButtonDown() override;
    bool doLeftButtonUp() override;
    bool doLeftDrag() override;
    bool doMouseMove() override;

    Vis::PathSetVisualisation _vis;

    Vis::PathView::Handle *_handle;
    bool _dragging;
    bool _initPlacement;

    void _snapHandle( const Vis::FV*, Vec3f&) const;
    bool _execLeftDrag();
    void _showPathInfo();
    void _updateCaption();

    PathsHandler();
};  // end class

}}   // end namespace

#endif
