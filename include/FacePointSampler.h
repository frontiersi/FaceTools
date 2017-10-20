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

/**
 * Densely samples the given face to extract N points having known positions
 * relative to one another on the face. Multiple faces can then be compared
 * by relating corresponding points to one another.
 */

#ifndef FACE_TOOLS_FACE_POINT_SAMPLER_H
#define FACE_TOOLS_FACE_POINT_SAMPLER_H

#include "ObjMetaData.h"

namespace FaceTools
{

class FaceTools_EXPORT FacePointSampler
{
public:
    // Requires pre-processed face from FacePreProcessor
    explicit FacePointSampler( const ObjMetaData::Ptr preProcessedFace);

    // Find the N sampled points on the mesh. Returns the actual number of points sampled
    // which will be N approx D*(H+1)/2 where D and H are the adjacent integers nearest
    // to giving N. Returns <= 0 if points can't be extracted for any reason. NB the points
    // that are extracted are the original vertices nearest to the sampling locations
    // so for accuracy, the given model should be a dense mesh.
    int extractPoints( int N);

    size_t getNumRings() const { return _crings.size();}
    size_t getNumRingPoints( int ringId) const; // ringId in [0,getNumRings())

private:
    const ObjMetaData::Ptr _omd;
    std::vector< std::vector<int> > _crings;    // Correspondence rings

    FacePointSampler( const FacePointSampler&); // No copy
    void operator=( const FacePointSampler&);   // No copy
};  // end class

}   // end namespace

#endif
