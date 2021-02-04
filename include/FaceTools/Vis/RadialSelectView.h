/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <r3d/Mesh.h>
#include "SphereView.h"
#include "SimpleView.h"
#include <vtkPolyLineSource.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT RadialSelectView : public SimpleView
{
public:
    RadialSelectView();
    ~RadialSelectView() override;

    void setVisible( bool, ModelViewer*) override;
    bool belongs( const vtkProp*) const override;
    void pokeTransform( const vtkMatrix4x4*) override;

    void setColour( double r, double g, double b, double a) override;

    void update( const Vec3f &c, const r3d::Mesh&, const std::list<int>&);

private:
    SphereView *_centre;
    vtkNew<vtkPolyLineSource> _loop;
    vtkActor *_loopActor;
};  // end class

}}   // end namespaces

#endif
