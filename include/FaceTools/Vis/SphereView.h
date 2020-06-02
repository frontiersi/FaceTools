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

#ifndef FACE_TOOLS_SPHERE_VIEW_H
#define FACE_TOOLS_SPHERE_VIEW_H

#include "ViewInterface.h"
#include <r3dvis/VtkScalingActor.h>
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT SphereView : public ViewInterface
{
public:
    SphereView( const Vec3f& centre=Vec3f(0,0,0), float radius=1.0, bool pickable=true, bool fixedScale=false);
    virtual ~SphereView();

    void setResolution( int);   // Default 8
    int resolution() const;

    void setPickable( bool);
    bool pickable() const;

    void setFixedScale( bool);
    bool fixedScale() const;

    void setScaleFactor( float);
    float scaleFactor() const;

    void setCentre( const Vec3f&);
    const Vec3f& centre() const;

    void setRadius( float);                            // Set radius
    float radius() const;                              // Get radius

    float opacity() const;

    void setCaption( const std::string&);               // Set caption used when highlighting.
    void setCaption( const QString&);
    std::string caption() const;

    void setCaptionColour( const QColor&);
    void showCaption( bool);

    const vtkProp* prop() const;     
    const double* colour() const;                       // Return a 3-tuple array for the rgb components.

    void setColour( double r, double g, double b, double a) override;
    void setVisible( bool, ModelViewer*) override;
    bool isVisible() const override { return _visible;}
    bool belongs( const vtkProp*) const override;
    void pokeTransform( const vtkMatrix4x4*) override;

    const vtkMatrix4x4* transform() const;

private:
    ModelViewer *_vwr;
    bool _visible;
    r3dvis::VtkScalingActor* _actor;
    vtkNew<vtkSphereSource> _source;
    vtkNew<vtkCaptionActor2D> _caption;
    void _init( const Vec3f&, float, bool, bool);
    void _updateCaptionPosition();
};  // end class

}}   // end namespaces

#endif
