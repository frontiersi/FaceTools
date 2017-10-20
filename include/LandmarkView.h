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

#ifndef FACE_TOOLS_LANDMARK_VIEW_H
#define FACE_TOOLS_LANDMARK_VIEW_H

#include "Landmarks.h"
#include "VisualisationOptions.h"
#include "ModelViewer.h"
#include "ModelViewerAnnotator.h"
#include <vtkSphereSource.h>
#include <vtkActor.h>

namespace FaceTools
{

class FaceTools_EXPORT LandmarkView
{
public:
    LandmarkView( ModelViewer*, const Landmarks::Landmark*, const VisualisationOptions::Landmarks&);
    virtual ~LandmarkView();

    void show( bool enable);    // Add/remove from viewer
    bool isShown() const;       // Returns true iff shown
    void update();              // Update from stored meta data
    void highlight( bool);      // Show highlighted and annotate on viewer

    void setVisualisationOptions( const VisualisationOptions::Landmarks&);

    // Returns true if this landmark is under the given coordinates.
    bool isPointedAt( const QPoint&) const;

private:
    ModelViewer* _viewer;
    ModelViewerAnnotator* _annotator;
    const Landmarks::Landmark* _landmark;
    vtkSmartPointer<vtkSphereSource> _source;
    vtkSmartPointer<vtkActor> _actor;
    bool _ishighlighted;
    VisualisationOptions::Landmarks _visopts;
    int _msgID;
    bool _isshown;
};  // end class

}   // end namespace

#endif
