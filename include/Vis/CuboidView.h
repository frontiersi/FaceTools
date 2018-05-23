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

#ifndef FACE_TOOLS_CUBOID_VIEW_H
#define FACE_TOOLS_CUBOID_VIEW_H

/**
 * Show a transparent cuboid around each component.
 */

#include <ModelViewer.h>
#include <Hashing.h>

namespace FaceTools {
class FaceModel;

namespace Vis {

class FaceTools_EXPORT CuboidView
{
public:
    CuboidView( const FaceModel*, float lineWidth=2.0f, float red=0.5f, float green=0.2f, float blue=1.0f);
    ~CuboidView();

    // Not pickable by default.
    void setPickable( bool);
    bool pickable() const;

    bool isVisible() const { return _visible;}
    void setVisible( bool, ModelViewer* viewer);

private:
    ModelViewer *_viewer;
    bool _visible;
    bool _pickable;
    std::vector<vtkSmartPointer<vtkActor> > _actors;   // The cuboid actors (one per component)

    CuboidView( const CuboidView&);     // No copy
    void operator=( const CuboidView&); // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
