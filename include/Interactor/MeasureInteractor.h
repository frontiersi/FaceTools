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

#ifndef FACE_TOOLS_MEASURE_INTERACTOR_H
#define FACE_TOOLS_MEASURE_INTERACTOR_H

/**
 * Manages mouse interactions for the defining of measurements between points by interfacing with a
 * provided SurfacePathSet which maintains the data for individual SurfacePath instances. Clients
 * should connect to the signals fired from this MeasureInstance. Visualisation of the paths
 * must be handled elsewhere.
 */

#include "ModelViewerInteractor.h"
#include <SurfacePathSet.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT MeasureInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    explicit MeasureInteractor( SurfacePathSet*);

signals:
    void onHover( SurfacePath::Handle*);            // Change in handle the mouse cursor is over.
    void onUpdate( SurfacePath::Handle*);           // May be an entirely new, or an updated path.
    void onRightButtonDown( SurfacePath::Handle*);  // Right button clicked down over a path handle.

private:
    void rightButtonDown( const QPoint&) override;
    void leftButtonDown( const QPoint&) override;
    void leftButtonUp( const QPoint&) override;
    void mouseMove( const QPoint&) override;
    void leftDrag( const QPoint&) override;

    SurfacePathSet* _paths;
    SurfacePath::Handle *_handle, *_hhandle;
    bool _lock;
};  // end class

}   // end namespace
}   // end namespace

#endif
