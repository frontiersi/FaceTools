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

#include <ModelViewer.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT LandmarkView
{
public:
    LandmarkView();
    virtual ~LandmarkView();

    void set( const std::string&, const cv::Vec3f&);    // Set displayed name (on highlight) and position
    void setPos( const cv::Vec3f&);                     // Update position of actor.
    cv::Vec3f pos() const;                              // Return this view's position.

    void setVisible( bool, ModelViewer* viewer);        // Add/remove from viewer
    bool isVisible() const;                             // Returns true iff shown

    void highlight( bool);                              // Show highlighted (only if already visible)
    bool isHighlighted() const;                         // Returns true if highlighted

    bool pointedAt( const QPoint&) const;               // Returns true if this landmark is under the given coordinates.
    bool isProp( const vtkProp*) const;                 // Returns true if given prop is this landmark's actor.
    const vtkProp* prop() const { return _actor;}

private:
    ModelViewer* _viewer;
    vtkSmartPointer<vtkSphereSource> _source;
    vtkSmartPointer<vtkActor> _actor;
    vtkSmartPointer<vtkCaptionActor2D> _caption;
    bool _ishighlighted;
    bool _isshown;

    LandmarkView( const LandmarkView&);     // No copy
    void operator=( const LandmarkView&);   // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
