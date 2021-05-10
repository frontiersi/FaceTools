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

#ifndef FACE_TOOLS_VIS_ASYMMETRY_VIEW_H
#define FACE_TOOLS_VIS_ASYMMETRY_VIEW_H

#include "SimpleView.h"
#include <vtkTransformPolyDataFilter.h>
#include <vtkArrowSource.h>
#include <vtkLineSource.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT AsymmetryView : public SimpleView
{
public:
    AsymmetryView();

    void update( const Vec3f &p, const Vec3f &q, const Vec3f &d);

    void setLineColour( double r, double g, double b, double a=-1);
    void setArrowColour( double r, double g, double b, double a=-1);

private:
    vtkNew<vtkTransformPolyDataFilter> _transformPD;
    vtkNew<vtkArrowSource> _arrow;
    vtkNew<vtkLineSource> _line;
    vtkActor *_arrowActor;
    vtkActor *_lineActor;
};  // end class

}}   // end namespaces

#endif
