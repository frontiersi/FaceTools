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

#ifndef FACE_TOOLS_BOUNDARY_VIEW_H
#define FACE_TOOLS_BOUNDARY_VIEW_H

#include "ModelViewer.h"

namespace FaceTools
{

class FaceTools_EXPORT BoundaryView
{
public:
    BoundaryView();
    virtual ~BoundaryView();

    void setVisible( bool enable, ModelViewer* viewer);
    bool isVisible() const;

    void setBoundary( const RFeatures::ObjModel::Ptr, const IntSet* bverts); // (re)create the boundary actor

private:
    ModelViewer *_viewer;
    bool _isshown;
    vtkSmartPointer<vtkActor> _boundary;

    BoundaryView( const BoundaryView&);   // No copy
    void operator=( const BoundaryView&); // No copy
};  // end class

}   // end namespace

#endif
