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

#include "ModelOptions.h"
#include "ModelViewer.h"
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>

namespace FaceTools
{

class FaceTools_EXPORT LandmarkView
{
public:
    explicit LandmarkView( const ModelOptions&);
    virtual ~LandmarkView();

    void highlight( bool);                              // Show highlighted (only if already visible)
    bool isVisible() const;                             // Returns true iff shown
    void set( const std::string&, const cv::Vec3f&);    // Set displayed name (on highlight) and position
    void setVisible( bool, ModelViewer* viewer);        // Add/remove from viewer

    void setOptions( const ModelOptions&);

    // Returns true if this landmark is under the given coordinates.
    bool isPointedAt( const QPoint&) const;

    // Returns true if given prop is this landmark's actor.
    bool isProp( const vtkProp*) const;

private:
    ModelViewer* _viewer;
    vtkSmartPointer<vtkSphereSource> _source;
    vtkSmartPointer<vtkActor> _actor;
    vtkSmartPointer<vtkCaptionActor2D> _caption;
    bool _ishighlighted;
    ModelOptions _opts;
    bool _isshown;

    LandmarkView( const LandmarkView&);     // No copy
    void operator=( const LandmarkView&);   // No copy
};  // end class

}   // end namespace

#endif
