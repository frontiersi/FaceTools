/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
using RFeatures::ObjModel;
#include <boost/foreach.hpp>
#include <cassert>

CurvatureVariableSpeedFunctor::CurvatureVariableSpeedFunctor( const ObjModelCurvatureMap::Ptr omcm)
{
    assert( omcm);
    const ObjModel::Ptr model = omcm->getObject();

    double kp1, kp2;
    const IntSet& vidxs = model->getVertexIds();
    BOOST_FOREACH ( const int& vi, vidxs)
    {
        omcm->getVertexPrincipalCurvature1( vi, kp1); // Magnitude of curvature in direction of max curvature
        omcm->getVertexPrincipalCurvature2( vi, kp2); // Magnitude of curvature in direction of minimum curvature
        _curvedness[vi] = sqrt( pow(kp1,2) + pow(kp2,2)/2);
    }   // end foreach
}   // end ctor
