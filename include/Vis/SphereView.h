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

#ifndef FACE_TOOLS_SPHERE_VIEW_H
#define FACE_TOOLS_SPHERE_VIEW_H

#include <ModelViewer.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT SphereView
{
public:
    SphereView( const cv::Vec3f& centre, double radius, bool pickable=true);
    virtual ~SphereView();

    void setResolution( int);   // Default 8
    int resolution() const;

    void setPickable( bool);
    bool pickable() const;

    void setOpacity( double);
    double opacity() const;

    void setCentre( const cv::Vec3f&);                  // Update position of actor.
    cv::Vec3f centre() const;                           // Return this view's position.

    void setRadius( double);                            // Set radius
    double radius() const;                              // Get radius

    void setVisible( bool, ModelViewer* viewer);        // Add/remove from viewer
    bool isVisible() const;                             // Returns true iff shown

    void setCaption( const std::string&);               // Set highlight caption
    void highlight( bool);                              // Show highlighted (only if already visible)
    bool isHighlighted() const;                         // Returns true if highlighted

    bool pointedAt( const QPoint&) const;               // Returns true if this actor under the given coordinates.
    bool isProp( const vtkProp*) const;                 // Returns true if given prop is this actor.
    const vtkProp* prop() const { return _actor;}

private:
    ModelViewer* _viewer;
    vtkSmartPointer<vtkSphereSource> _source;
    vtkSmartPointer<vtkActor> _actor;
    vtkSmartPointer<vtkCaptionActor2D> _caption;
    bool _ishighlighted;
    bool _isshown;

    SphereView( const SphereView&);     // No copy
    void operator=( const SphereView&); // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
