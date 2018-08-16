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

#ifndef FACE_TOOLS_BOUNDING_VIEW_H
#define FACE_TOOLS_BOUNDING_VIEW_H

/**
 * Show a transparent cuboid around each component.
 */

#include <ModelViewer.h>
#include <Hashing.h>
#include <vtkCubeSource.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT BoundingView
{
public:
    BoundingView( const std::vector<cv::Vec6d>& bounds, float lineWidth=1.0f, float red=0.6f, float green=0.3f, float blue=0.8f);
    ~BoundingView();

    // Not pickable by default.
    void setPickable( bool);
    bool pickable() const;

    void setVisible( bool, ModelViewer*);
    bool visible() const { return _visible;}

    void setHighlighted( bool);         // Set all components highlighted or not.
    void setHighlighted( int, bool);    // Set the given component to be highlighted or not.

    void pokeTransform( const vtkMatrix4x4*);

    // New bounds vector must be same length as existing # sources!
    void updateBounds( const std::vector<cv::Vec6d>&);

private:
    ModelViewer *_viewer;
    bool _visible;
    bool _pickable;
    float _lw;
    cv::Vec3f _colour;
    std::vector<vtkCubeSource*> _sources;
    std::vector<vtkActor*> _actors;   // The associated cuboid actors (one per component)

    BoundingView( const BoundingView&) = delete;
    void operator=( const BoundingView&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
