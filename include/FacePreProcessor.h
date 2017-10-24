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
 * Preprocesses face model data prior to use in other algorithms.
 * Object meta data must already have face detected landmarks.
 */

#ifndef FACE_TOOLS_FACE_PRE_PROCESSOR_H
#define FACE_TOOLS_FACE_PRE_PROCESSOR_H

#include "ObjMetaData.h"

namespace FaceTools
{

class FaceTools_EXPORT FacePreProcessor
{
public:
    // Face detection must already have been carried out on the given model.
    // Must have landmark FaceTools::Landmarks::NASAL_TIP and the L_EYE_CENTRE
    // and R_EYE_CENTRE landmarks defined. Orientation vectors must be set.
    explicit FacePreProcessor( ObjMetaData::Ptr);

    // Performs the following in order:
    // 1) Fills holes (not the outer boundary).
    // 2) Sets triangles to be no larger than mta units in area (subdivides with extra vertices).
    // 3) Crops around face using standardised radius based on multiple of nose length (nl).
    // 4) Smooths the face with smooth curvature factor sf over max of 10 iterations.
    // 5) Transforms the "centre" of the face to the world coordinate origin and orients.
    // 6) Shifts landmarks to the closest positions on the newly smoothed face.
    // Leaving the parameters as default simply transforms to standard position (5&6).
    void operator()( bool fillHoles=false, double mta=0, double nl=0, double sf=0);

private:
    ObjMetaData::Ptr _omd;
    FacePreProcessor( const FacePreProcessor&); // No copy
    void operator=( const FacePreProcessor&);   // No copy
};  // end class

}   // end namespace

#endif
