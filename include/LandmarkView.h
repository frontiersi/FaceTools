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
#include "ModelOptions.h"
#include "ModelViewer.h"
#include <vtkSphereSource.h>
#include <vtkActor.h>

namespace FaceTools
{

class FaceTools_EXPORT LandmarkView
{
public:
    LandmarkView( const Landmarks::Landmark*, const ModelOptions::Landmarks&);
    virtual ~LandmarkView();

    void setVisible( bool, ModelViewer* viewer);   // Add/remove from viewer
    bool isVisible() const;                 // Returns true iff shown
    void update();                          // Update from stored meta data
    void highlight( bool);                  // Show highlighted (only if already visible)

    void setOptions( const ModelOptions::Landmarks&);

    // Returns true if this landmark is under the given coordinates.
    bool isPointedAt( const QPoint&) const;

    // Returns true if given prop is this landmark's actor.
    bool isProp( const vtkProp*) const;

private:
    ModelViewer* _viewer;
    const Landmarks::Landmark* _landmark;
    vtkSmartPointer<vtkSphereSource> _source;
    vtkSmartPointer<vtkActor> _actor;
    bool _ishighlighted;
    ModelOptions::Landmarks _opts;
    bool _isshown;

    LandmarkView( const LandmarkView&);     // No copy
    void operator=( const LandmarkView&);   // No copy
};  // end class

}   // end namespace

#endif
