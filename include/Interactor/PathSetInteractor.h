/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include "ModelEntryExitInteractor.h"
#include <PathSetVisualisation.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT PathSetInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    PathSetInteractor( MEEI*, Vis::PathSetVisualisation*, QStatusBar *sbar=nullptr);

    // Get the ID of the path associated with the handle being hovered over (if any).
    // Returns -1 if no handle hovered over. Use hoverModel() to get associated model.
    int hoverPathId() const { return _handle ? _handle->pathId() : -1;}
    Vis::FV* hoverModel() const { return _meei->model();}

    void setPathDrag( int pid);
    bool moveDragHandle( const cv::Vec3f&);
    void setCaptionInfo( const FM*, int);

public slots:
    void doOnEnterHandle( const Vis::FV*, const vtkProp*);
    void doOnLeaveHandle( const Vis::FV*);

private:
    bool leftButtonDown( const QPoint&) override;
    bool leftButtonUp( const QPoint&) override;
    bool leftDrag( const QPoint&) override;
    bool mouseMove( const QPoint&) override;
    void onEnabledStateChanged(bool) override;

    MEEI *_meei;
    Vis::PathSetVisualisation *_vis;
    bool _drag; // True if path currently being dragged
    Vis::PathView::Handle* _handle;
    Vis::FV* _view;

    static const QString s_msg0;
    static const QString s_msg1;
};  // end class

}}   // end namespace

#endif
