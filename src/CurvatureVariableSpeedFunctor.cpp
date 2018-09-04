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

#include <CurvatureVariableSpeedFunctor.h>
using FaceTools::CurvatureVariableSpeedFunctor;
using RFeatures::ObjModelCurvatureMap;

CurvatureVariableSpeedFunctor::CurvatureVariableSpeedFunctor( const ObjModelCurvatureMap::Ptr omcm)
{
    double kp1, kp2;
    for ( int vi : omcm->model()->getVertexIds())
    {
        omcm->vertexPC1( vi, kp1); // Magnitude of curvature in direction of max curvature
        omcm->vertexPC2( vi, kp2); // Magnitude of curvature in direction of minimum curvature
        _curvedness[vi] = sqrt( pow(kp1,2) + pow(kp2,2)/2);
    }   // end for
}   // end ctor
