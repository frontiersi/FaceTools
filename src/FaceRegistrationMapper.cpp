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

#include <FaceRegistrationMapper.h>
using FaceTools::FaceRegistrationMapper;
using FaceTools::ObjMetaData;
using RFeatures::ObjModel;
#include <FaceTools.h>


FaceRegistrationMapper::FaceRegistrationMapper( ObjMetaData::Ptr omd) : _omd(omd) {}


void FaceRegistrationMapper::generate()
{
    using namespace RFeatures;
    // Crop around point between the eyes (in XY plane of eyes) and immediately behind nose tip.
    ObjModel::Ptr cmodel = FaceTools::cropAroundFaceCentre( _omd, 2.2);
    // Increase vertex density to max of 2mm separation
    ObjModelVertexAdder::addVertices( cmodel, 2.0);

    // Super-smooth
    ObjModelCurvatureMap::Ptr cmap = ObjModelCurvatureMap::create( cmodel, 0);
    size_t nSmoothOps = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( 0.001, nSmoothOps);
}   // end generate
