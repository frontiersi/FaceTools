/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include "SimpleView.h"
#include <vtkCubeSource.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT BoundingView : public SimpleView
{
public:
    BoundingView( const FM*, float lineWidth);

    void pokeTransform( const vtkMatrix4x4*) override;

protected:
    void pokeTransform( vtkActor*, const vtkMatrix4x4*) override;

private:
    const FM* _fm;
    vtkNew<vtkCubeSource> _cubeSource;
};  // end class

}}   // end namespaces

#endif
