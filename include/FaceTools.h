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

// Get all boundary loops on the given object.
FaceTools_EXPORT int findBoundaryLoops( const RFeatures::ObjModel::Ptr, std::list<std::vector<cv::Vec3f> > &loops);

// Find the boundaries of the given model and create and return a new model being
// the single component that is contiguously connected by polygon edges to vidx.
FaceTools_EXPORT RFeatures::ObjModel::Ptr getComponent( const RFeatures::ObjModel::Ptr, int vidx);

// Checks if the essential landmarks (eyes and nose tip) needed for crop,
// calcFaceCropRadius, transformToOrigin and others are present in the given set of landmarks.
FaceTools_EXPORT bool hasReqLandmarks( const LandmarkSet&);

// Crop and copy the given model to be the part connected to vertex svid with a boundary of R from centre v.
// The choice of svid allows the cropped part to be inside or outside the boundary. If svid is not a valid
// model vertex, an empty object is returned. Returned object is parsable as a single connected component.
FaceTools_EXPORT RFeatures::ObjModel::Ptr crop( const RFeatures::ObjModel::Ptr, const cv::Vec3f& v, int svid=0, double R=DBL_MAX);

// As above, but crop from a given set of polygons.
FaceTools_EXPORT RFeatures::ObjModel::Ptr crop( const RFeatures::ObjModel::Ptr, const IntSet& fids);

// Calculate and return the centre of the face as the point directly behind the nose tip in the plane of the front of the eyes.
// Requires the centre points of the eyes, the nose tip, and the *up* vector defining the proper upright position of the face.
FaceTools_EXPORT cv::Vec3f calcFaceCentre( const cv::Vec3f& upVector, const cv::Vec3f& leye, const cv::Vec3f& reye, const cv::Vec3f& ntip);

// Calculate cropping radius for a face as G times the distance from the face centre to the point halfway between the eyes.
FaceTools_EXPORT double calcFaceCropRadius( const cv::Vec3f& faceCentre, const cv::Vec3f& leye, const cv::Vec3f& reye, double G);

// Create a vertices only ObjModel from the given row of points.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromVertices( const cv::Mat_<cv::Vec3f>& row);

// Create a vertices only ObjModel from the given subset of vertices.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromSubset( const RFeatures::ObjModel::Ptr, const IntSet& vidxs);

// Given a source model and a subset of vertices (vidxs), create and return a new
// points only model consisting of the given vertices transformed using matrix T.
// On return, if newVidxsToOld is not null, it is set with vertex mappings of the
// new (returned) object to the given vertex indices on the source model (vidxs).
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromTransformedSubset( const RFeatures::ObjModel::Ptr source, const IntSet& vidxs,
                                                                       const cv::Matx44d& T, std::unordered_map<int,int>* newVidxsToOld=NULL);

// Flatten the source face to be in the XY plane, and set entries in provided map (if given) to key the source vertices
// from the new vertices in the returned (flattened) model.
FaceTools_EXPORT RFeatures::ObjModel::Ptr makeFlattened( const RFeatures::ObjModel::Ptr source, std::unordered_map<int,int>* newVidxsToOld=NULL);

// Clean the given model to ensure it's a triangulated manifold (wraps RFeatures::ObjModelCleaner).
FaceTools_EXPORT void clean( RFeatures::ObjModel::Ptr);

// Returns NULL if cannot load or if can't clean (if doClean true).
FaceTools_EXPORT RFeatures::ObjModel::Ptr loadModel( const std::string& fname,
                                                     bool loadTexture=false, bool doClean=false);

}   // end namespace

#endif
