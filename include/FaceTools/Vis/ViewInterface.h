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

#ifndef FACE_TOOLS_VIEW_INTERFACE_H
#define FACE_TOOLS_VIEW_INTERFACE_H

#include <FaceTools/FaceTypes.h>
#include <FaceTools/ModelViewer.h>
#include <vtkProp.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT ViewInterface
{
public:
    virtual void setColour( double r, double g, double b, double a) = 0;
    virtual void setVisible( bool, ModelViewer*) = 0;
    virtual bool isVisible() const = 0;
    virtual bool belongs( const vtkProp*) const = 0;
    virtual void pokeTransform( const vtkMatrix4x4*) = 0;
};  // end class

}}   // end namespaces

#endif
