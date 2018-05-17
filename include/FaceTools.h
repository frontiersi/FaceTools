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

#ifndef FACE_TOOLS_H
#define FACE_TOOLS_H

#include "LandmarkSet.h"
#include "MiscFunctions.h"

namespace FaceTools {

// Checks if the essential landmarks (eyes and nose tip) needed for crop,
// calcFaceCropRadius, transformToOrigin and others are present in the given set of landmarks.
FaceTools_EXPORT bool hasReqLandmarks( const LandmarkSet&);

// Calculate and return the centre of the face as the point directly behind the nose tip in the plane of the front of the eyes.
// Requires the centre points of the eyes, the nose tip, and the *up* vector defining the proper upright position of the face.
FaceTools_EXPORT cv::Vec3f calcFaceCentre( const cv::Vec3f& upVector, const cv::Vec3f& leye, const cv::Vec3f& reye, const cv::Vec3f& ntip);

// Calculate cropping radius for a face as G times the distance from the face centre to the point halfway between the eyes.
FaceTools_EXPORT double calcFaceCropRadius( const cv::Vec3f& faceCentre, const cv::Vec3f& leye, const cv::Vec3f& reye, double G);

// Create a vertices only ObjModel from the given row of points.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromVertices( const cv::Mat_<cv::Vec3f>& row);

// Create a vertices only ObjModel from the given subset of vertices.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromSubset( const RFeatures::ObjModel*, const IntSet& vidxs);

// Given a source model and a subset of vertices (vidxs), create and return a new
// points only model consisting of the given vertices transformed using matrix T.
// On return, if newVidxsToOld is not null, it is set with vertex mappings of the
// new (returned) object to the given vertex indices on the source model (vidxs).
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromTransformedSubset( const RFeatures::ObjModel* source, const IntSet& vidxs,
                                                                       const cv::Matx44d& T, std::unordered_map<int,int>* newVidxsToOld=NULL);

// Flatten the source face to be in the XY plane, and set entries in provided map (if given) to key the source vertices
// from the new vertices in the returned (flattened) model.
FaceTools_EXPORT RFeatures::ObjModel::Ptr makeFlattened( const RFeatures::ObjModel* source, std::unordered_map<int,int>* newVidxsToOld=NULL);

}   // end namespace

#endif
