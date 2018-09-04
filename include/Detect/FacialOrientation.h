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

#ifndef FACE_TOOLS_DETECT_FACIAL_ORIENTATION_H
#define FACE_TOOLS_DETECT_FACIAL_ORIENTATION_H

#include "FaceTools_Export.h"
#include <ObjModelKDTree.h>         // RFeatures

namespace FaceTools {
namespace Detect {

// Estimate the up and normal vectors using an interative approach that evaluates
// normals along line segments measured over portions of the face. The left and right
// positions v0 and v1 should be given by the surface vertices at the left and right pupils.
FaceTools_EXPORT bool findOrientation( const RFeatures::ObjModelKDTree::Ptr kdt,
                                       const cv::Vec3f& v0,    // Position of left eye
                                       const cv::Vec3f& v1,    // Position of right eye
                                       cv::Vec3f& vnorm,       // Output face norm
                                       cv::Vec3f& vup);        // Output up vector

// Calculate an up vector from the difference vectors e0-nb and e1-nb where nb is
// given as the nose bridge, e0 the left eye, and e1 the right eye.
cv::Vec3d calcUp( const cv::Vec3f& nb, const cv::Vec3f& e0, const cv::Vec3f& e1);

// Calculate the normal from the up vector returned from calcUp and the eye positions.
cv::Vec3d calcNormal( const cv::Vec3f& upv, const cv::Vec3f& e0, const cv::Vec3f& e1);

}   // end namespace
}   // end namespace

#endif
