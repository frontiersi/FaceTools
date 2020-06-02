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

#ifndef FACE_TOOLS_PATHS_HANDLER_H
#define FACE_TOOLS_PATHS_HANDLER_H

#include "PropHandler.h"
#include <FaceTools/Vis/PathSetVisualisation.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT PathsHandler : public PropHandler
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<PathsHandler>;
    static Ptr create();

    void refreshState() override;

    // For snapping to visible landmarks.
    void setLandmarksVisualisation( const Vis::BaseVisualisation *vis) { _lmkVis = vis;}

    Vis::PathSetVisualisation &visualisation() { return _vis;}

    // The path being hovered over (if any).
    Vis::PathView::Handle* hoverPath() const { return _handle;}

    void addPath( const Vis::FV*, int pathId);

    void leavePath();

signals:
    void onStartedDrag( Vis::PathView::Handle*);
    void onFinishedDrag( Vis::PathView::Handle*);

private slots:
    void doEnterProp( Vis::FV*, const vtkProp*) override;
    void doLeaveProp( Vis::FV*, const vtkProp*) override;

private:
    bool leftButtonDown() override;
    bool leftButtonUp() override;
    bool leftDrag() override;
    bool mouseMove() override;

    Vis::PathSetVisualisation _vis;

    Vis::PathView::Handle *_handle;
    bool _dragging;
    bool _initPlacement;
    const Vis::BaseVisualisation *_lmkVis;

    void _enterPath();

    PathsHandler();
};  // end class

}}   // end namespace

#endif
