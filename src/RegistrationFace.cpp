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

#include <RegistrationFace.h>
#include <ObjModelTools.h>
#include <CurvatureVariableSpeedFunctor.h>  // FaceTools
#include <CurvatureSpeedFunctor.h>          // FaceTools
using FaceTools::RegistrationFace;
using namespace RFeatures;


// public
RegistrationFace::RegistrationFace( const ObjModelCurvatureMap::Ptr cm) : _curvmap(cm)
{
    const ObjModel::Ptr model = cm->getObject();
    // Create the per polygon internal angles
    ObjModelTriangleMeshParser parser( model);
    ObjModelFaceAngleCalculator faceAngleCalculator( model);
    parser.addTriangleParser( &faceAngleCalculator);
    parser.parse();
    _faceAngles = faceAngleCalculator.getFaceAngles();
}   // end ctor


// private
ObjModel::Ptr RegistrationFace::sample( int n, ObjModelFastMarcher::SpeedFunctor* sfunctor)
{
    const ObjModel::Ptr model = _curvmap->getObject();
    ObjModelRemesher remesher( model, sfunctor, &_faceAngles);
    remesher.sample( *model->getVertexIds().begin(), n);
    _sedges.clear();
    remesher.createSaddleEdges( _sedges);
    return remesher.getSampleObject();
}   // end sample


// private
ObjModel::Ptr RegistrationFace::sampleInterp( int n, ObjModelFastMarcher::SpeedFunctor* sfunctor)
{
    const ObjModel::Ptr model = _curvmap->getObject();
    ObjModelRemesher remesher( model, sfunctor, &_faceAngles);
    remesher.sampleInterpolated( *model->getVertexIds().begin(), n);
    _sedges.clear();
    remesher.createSaddleEdges( _sedges);
    return remesher.getSampleObject();
}   // end sampleInterp


// public
ObjModel::Ptr RegistrationFace::sampleHighCurvature( int n)
{
    FaceTools::CurvatureSpeedFunctor sfunctor( _curvmap);
    return sample( n, &sfunctor);
}   // end sampleHighCurvature


// public
ObjModel::Ptr RegistrationFace::sampleUniformly( int n)
{
    ObjModelFastMarcher::SpeedFunctor sfunctor;
    return sample( n, &sfunctor);
}   // end sampleUniformly


// public
ObjModel::Ptr RegistrationFace::sampleUniformlyInterpolated( int n)
{
    ObjModelFastMarcher::SpeedFunctor sfunctor;
    return sampleInterp( n, &sfunctor);
}   // end sampleUniformlyInterpolated


// public
ObjModel::Ptr RegistrationFace::sampleCurvatureVariableInterpolated( int n)
{
    FaceTools::CurvatureVariableSpeedFunctor sfunctor( _curvmap);
    return sampleInterp( n, &sfunctor);
}   // end sampleCurvatureVariableInterpolated
