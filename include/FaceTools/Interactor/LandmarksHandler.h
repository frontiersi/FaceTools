/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_LANDMARKS_HANDLER_H
#define FACE_TOOLS_LANDMARKS_HANDLER_H

#include "GizmoHandler.h"
#include <FaceTools/Vis/LandmarksVisualisation.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT LandmarksHandler : public GizmoHandler
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<LandmarksHandler>;
    static Ptr create();

    void refresh() override;

    Vis::LandmarksVisualisation &visualisation() { return _vis;}
    const Vis::LandmarksVisualisation &visualisation() const { return _vis;}

    // Return the ID of the landmark that's currently being moved
    // and set out parameter lat to the FaceSide of the landmark.
    int dragging( FaceSide& lat) const { lat = _lat; return _dragId;}

    void setEmitOnDrag( bool v) { _emitDragUpdates = v;}

signals:
    void onStartedDrag( int, FaceSide) const;
    void onDoingDrag( int, FaceSide) const;
    void onFinishedDrag( int, FaceSide) const;

    void onEnterLandmark( int, FaceSide) const;
    void onLeaveLandmark( int, FaceSide) const;

private:
    bool doEnterProp() override;
    bool doLeaveProp() override;
    bool doLeftButtonDown() override;
    bool doLeftButtonUp() override;
    bool doLeftDrag() override;

    Vis::LandmarksVisualisation _vis;
    int _hoverId;
    int _dragId;
    FaceSide _lat;
    bool _emitDragUpdates;

    void _leaveLandmark();

    LandmarksHandler();
};  // end class

}}   // end namespaces

#endif
