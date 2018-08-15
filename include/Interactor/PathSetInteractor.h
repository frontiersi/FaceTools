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

#ifndef FACE_TOOLS_PATH_SET_INTERACTOR_H
#define FACE_TOOLS_PATH_SET_INTERACTOR_H

/**
 * Manages mouse interactions for the defining of measurements between points by interfacing with a
 * provided PathSetSet which maintains the data for individual PathSet instances. Clients
 * should connect to the signals fired from this MeasureInstance. Visualisation of the paths
 * must be handled elsewhere.
 */

#include "FaceHoveringInteractor.h"
#include <PathSetVisualisation.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT PathSetInteractor : public FaceHoveringInteractor
{ Q_OBJECT
public:
    PathSetInteractor( FEEI*, Vis::PathSetVisualisation*, QStatusBar *sbar=nullptr);

    // Get the ID of the path associated with the handle being hovered over (if any).
    // Returns -1 if no handle hovered over. Use hoverModel() to get associated model.
    int hoverID() const;

    // Add a path on the hover model at the given coords. Client should check for
    // existing paths first before deciding to create a new one. Returns -1 if no path
    // could be created (e.g. if the given coords are not on the model).
    int addPath( const QPoint&);

    // Deletes the path returned by hoverID returning true on success.
    bool deletePath();

private:
    bool leftButtonDown( const QPoint&) override;
    bool leftButtonUp( const QPoint&) override;
    bool mouseMove( const QPoint&) override;
    bool leftDrag( const QPoint&) override;

    Vis::PathSetVisualisation *_vis;
    const Vis::PathView::Handle *_hdrag;
    const Vis::PathView::Handle *_hhover;
    cv::Vec3f _origPos;

    const Vis::PathView::Handle* getPathViewHandle( const QPoint&) const;
};  // end class

}   // end namespace
}   // end namespace

#endif
