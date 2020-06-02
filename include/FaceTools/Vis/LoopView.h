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

#ifndef FACE_TOOLS_LOOP_VIEW_H
#define FACE_TOOLS_LOOP_VIEW_H

#include "SimpleView.h"
#include <vtkPolyLineSource.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LoopView : public SimpleView
{
public:
    LoopView();

    void update( const std::vector<const Vec3f*>&);
    void update( const std::vector<Vec3f>&);

private:
    vtkNew<vtkPolyLineSource> _loop;
    vtkNew<vtkPoints> _points;
};  // end class

}}   // end namespaces

#endif
