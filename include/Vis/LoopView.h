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

#ifndef FACE_TOOLS_LOOP_VIEW_H
#define FACE_TOOLS_LOOP_VIEW_H

#include "SimpleView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LoopView : public SimpleView
{
public:
    LoopView( const RFeatures::ObjModel&, double lineWidth=1.0);

    void add( const std::list<int>& vidxs, double red=1.0, double green=1.0, double blue=1.0, double alpha=1.0);

private:
    const RFeatures::ObjModel& _model;
    const double _lw;
};  // end class

}}   // end namespaces

#endif
