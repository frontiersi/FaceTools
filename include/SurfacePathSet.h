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

#ifndef FACE_TOOLS_SURFACE_PATH_SET_H
#define FACE_TOOLS_SURFACE_PATH_SET_H

#include "SurfacePath.h"

namespace FaceTools {

class FaceTools_EXPORT SurfacePathSet
{
public:
    explicit SurfacePathSet( const FaceControl*);
    virtual ~SurfacePathSet();

    const FaceControl* faceControl() const { return _fcont;}

    SurfacePath::Handle* path( const QPoint&) const;        // Get handle of path pointed at, or NULL if none pointed at.
    SurfacePath::Handle* createPath( const QPoint&);        // Create new path with handles at given point, returning second handle.
    void updatePath( SurfacePath::Handle*, const QPoint&);  // Update given path handle to new position.
    void deletePath( SurfacePath*);                         // Delete given path.

private:
    const FaceControl* _fcont;
    std::unordered_set<SurfacePath*> _paths;
    std::unordered_map<const vtkProp*, SurfacePath::Handle*> _handlesMap;

    SurfacePathSet( const SurfacePathSet&); // No copy
    void operator=( const SurfacePathSet&); // No copy
};  // end class

}   // end namespace

#endif
