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

#ifndef FACE_TOOLS_LANDMARKS_HANDLER_H
#define FACE_TOOLS_LANDMARKS_HANDLER_H

#include "PropHandler.h"
#include "../Vis/LandmarksVisualisation.h"

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT LandmarksHandler : public PropHandler
{ Q_OBJECT
public:
    explicit LandmarksHandler( Vis::LandmarksVisualisation&);

    // Return the ID of the landmark that's currently being moved
    // and set out parameter lat to the FaceLateral of the landmark.
    int dragging( FaceLateral& lat) const { lat = _lat; return _drag;}

signals:
    void onStartedDrag( int);
    void onFinishedDrag( int);

private slots:
    void doEnterProp( Vis::FV*, const vtkProp*) override;
    void doLeaveProp( Vis::FV*, const vtkProp*) override;

private:
    bool leftButtonDown() override;
    bool leftButtonUp() override;
    bool leftDrag() override;

    Vis::LandmarksVisualisation &_vis;
    int _drag, _hover;  // IDs of the landmarks being dragged and hovered over.
    cv::Vec3f _dpos;
    FaceLateral _lat;

    // Landmark events are always for the currently selected model.
    void _landmarkMove( int, FaceLateral, const cv::Vec3f&);
    void _enterLandmark( int, FaceLateral);
    void _leaveLandmark( int, FaceLateral);
};  // end class

}}   // end namespaces

#endif
