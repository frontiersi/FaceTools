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

#ifndef FACE_TOOLS_VIS_MASK_VIEW_H
#define FACE_TOOLS_VIS_MASK_VIEW_H

#include "SimpleView.h"
#include <vtkPlaneSource.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT MaskView : public SimpleView
{
public:
    explicit MaskView( const r3d::Mesh&);

    void refresh( const FV*);
};  // end class

}}   // end namespaces

#endif
