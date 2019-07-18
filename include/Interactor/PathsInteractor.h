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

#ifndef FACE_TOOLS_PATHS_INTERACTOR_H
#define FACE_TOOLS_PATHS_INTERACTOR_H

#include "FaceViewInteractor.h"
#include <PathSetVisualisation.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT PathsInteractor : public FaceViewInteractor
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<PathsInteractor>;
    static Ptr create( Vis::PathSetVisualisation&);

    // The path being hovered over (if any).
    Vis::PathView::Handle* hoverPath() const { return _handle;}

    void setPathDrag( const Vis::FV*, int);

    void leavePath();   // On return, _handle is null.

    void setCaption( const Vis::FV*, int pathId);

signals:
    void onUpdated( Vis::PathView::Handle*);
    void onEnterPath( Vis::PathView::Handle*);
    void onLeavePath( Vis::PathView::Handle*);

protected:
    void enterProp( Vis::FV*, const vtkProp*) override;
    void leaveProp( Vis::FV*, const vtkProp*) override;

    bool leftButtonDown() override;
    bool leftButtonUp() override;
    bool leftDrag() override;

    bool mouseMove() override;

private:
    Vis::PathSetVisualisation &_vis;
    bool _drag; // True if path currently being dragged
    Vis::PathView::Handle* _handle;
    PathsInteractor( Vis::PathSetVisualisation&);
    void enterPath( const Vis::FV*, Vis::PathView::Handle*);
    void movePath( const Vis::FV*, const cv::Vec3f&);
};  // end class

}}   // end namespace

#endif
