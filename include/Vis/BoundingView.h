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

#ifndef FACE_TOOLS_BOUNDING_VIEW_H
#define FACE_TOOLS_BOUNDING_VIEW_H

#include <FaceTypes.h>
#include <ModelViewer.h>
#include <vtkCubeSource.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT BoundingView
{
public:
    BoundingView( const RFeatures::ObjModelBounds&, float lineWidth);
    ~BoundingView();

    void setColour( float r, float g, float b);

    // Not pickable by default.
    void setPickable( bool);
    bool pickable() const;

    void setVisible( bool, ModelViewer*);
    bool visible() const { return _visible;}

    void pokeTransform( vtkMatrix4x4*);

private:
    ModelViewer *_viewer;
    bool _visible;
    float _lw;
    vtkNew<vtkCubeSource> _cubeSource;
    vtkNew<vtkActor> _cubeActor;

    BoundingView( const BoundingView&) = delete;
    void operator=( const BoundingView&) = delete;
};  // end class

} }   // end namespace

#endif
