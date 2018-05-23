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

#ifndef FACE_TOOLS_OUTLINES_VIEW_H
#define FACE_TOOLS_OUTLINES_VIEW_H

/**
 * Creates outline actors for a model's boundaries.
 */

#include <ModelViewer.h>
#include <Hashing.h>
#include <ObjModelBoundaryFinder.h>   // RFeatures

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT OutlinesView
{
public:
    OutlinesView( const RFeatures::ObjModelBoundaryFinder&,
            float lineWidth=4.0f, float red=1.0f, float green=0.2f, float blue=0.2f);
    ~OutlinesView();

    bool isVisible() const { return _visible;}
    void setVisible( bool, ModelViewer* viewer);

    void transform( const vtkMatrix4x4*);

private:
    ModelViewer *_viewer;
    bool _visible;
    std::unordered_set<vtkSmartPointer<vtkActor> > _actors;   // The line actors forming the boundary

    OutlinesView( const OutlinesView&);     // No copy
    void operator=( const OutlinesView&);   // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif