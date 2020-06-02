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

#ifndef FACE_TOOLS_LANDMARKS_HANDLER_H
#define FACE_TOOLS_LANDMARKS_HANDLER_H

#include "PropHandler.h"
#include <FaceTools/Vis/LandmarksVisualisation.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT LandmarksHandler : public PropHandler
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<LandmarksHandler>;
    static Ptr create();

    void refreshState() override;

    Vis::LandmarksVisualisation &visualisation() { return _vis;}

    // Return the ID of the landmark that's currently being moved
    // and set out parameter lat to the FaceLateral of the landmark.
    int dragging( FaceLateral& lat) const { lat = _lat; return _dragId;}

signals:
    void onStartedDrag( int, FaceLateral);
    void onDoingDrag( int, FaceLateral);
    void onFinishedDrag( int, FaceLateral);

    void onEnterLandmark( int, FaceLateral);
    void onLeaveLandmark( int, FaceLateral);

private slots:
    void doEnterProp( Vis::FV*, const vtkProp*) override;
    void doLeaveProp( Vis::FV*, const vtkProp*) override;

private:
    bool leftButtonDown() override;
    bool leftButtonUp() override;
    bool leftDrag() override;

    Vis::LandmarksVisualisation _vis;

    int _hoverId;
    int _dragId;
    FaceLateral _lat;

    void _leaveLandmark();

    LandmarksHandler();
};  // end class

}}   // end namespaces

#endif
