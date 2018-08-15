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
#include <VtkScalingActor.h>    // QTools
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT SphereView
{
public:
    SphereView( const cv::Vec3f& centre, double radius, bool pickable=true, bool fixedScale=false);
    virtual ~SphereView();

    void setResolution( int);   // Default 8
    int resolution() const;

    void setPickable( bool);
    bool pickable() const;

    void setFixedScale( bool);
    bool fixedScale() const;

    void setScaleFactor( double);
    double scaleFactor() const;

    void setCentre( const cv::Vec3f&);                  // Update position of actor.
    const cv::Vec3f& centre() const;                    // Return this view's position.

    void setRadius( double);                            // Set radius
    double radius() const;                              // Get radius

    void setOpacity( double);
    double opacity() const;

    void setColour( double r, double g, double b);      // Set colour as rgb components in [0,1].
    const double* colour() const;                       // Return a 3-tuple array for the rgb components.

    void setCaption( const std::string&);               // Set caption used when highlighting.
    void setHighlighted( bool);                         // Show the caption (only if already visible).

    void setVisible( bool, ModelViewer*);               // Set visibility of actors.
    bool visible() const { return _visible;}

    const vtkProp* prop() const;     

    void pokeTransform( const vtkMatrix4x4*);
    void fixTransform();                                // Fixes transform to Identity

private:
    bool _visible;
    QTools::VtkScalingActor* _actor;
    vtkNew<vtkSphereSource> _source;
    vtkNew<vtkCaptionActor2D> _caption;

    SphereView( const SphereView&) = delete;
    void operator=( const SphereView&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
