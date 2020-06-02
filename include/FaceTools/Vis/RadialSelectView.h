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

#ifndef FACE_TOOLS_VIS_RADIAL_SELECT_VIEW_H
#define FACE_TOOLS_VIS_RADIAL_SELECT_VIEW_H

#include "SimpleView.h"
#include <vtkPolyLineSource.h>
#include <vtkSphereSource.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT RadialSelectView : public SimpleView
{
public:
    RadialSelectView();

    bool belongs( const vtkProp*) const override;

    void setColour( double r, double g, double b, double a) override;

    void update( const Vec3f &p, const std::vector<const Vec3f*>&);

private:
    vtkNew<vtkSphereSource> _centre;
    vtkNew<vtkPolyLineSource> _loop;
    vtkActor *_centreActor;
    vtkActor *_loopActor;
};  // end class

}}   // end namespaces

#endif
