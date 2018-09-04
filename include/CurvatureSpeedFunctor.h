/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_CURVATURE_SPEED_FUNCTOR_H
#define FACE_TOOLS_CURVATURE_SPEED_FUNCTOR_H

#include "FaceTools_Export.h"
#include <ObjModelTools.h>

namespace FaceTools {

class FaceTools_EXPORT CurvatureSpeedFunctor : public RFeatures::ObjModelFastMarcher::SpeedFunctor
{
public:
    CurvatureSpeedFunctor( const RFeatures::ObjModelCurvatureMap::Ptr omcm);

    double operator()( int vidx) const override { return _curvedness.at(vidx);}

private:
    std::unordered_map<int, double> _curvedness;
};  // end class

}   // end namespace

#endif
