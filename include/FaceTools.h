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

#ifndef FACE_TOOLS_H
#define FACE_TOOLS_H

#include "LandmarkSet.h"
#include "MiscFunctions.h"
#include <Viewer.h> // RVTK

namespace FaceTools {

// Are the nasal root and subnasale landmarks present?
FaceTools_EXPORT bool hasCentreLandmarks( const Landmark::LandmarkSet&);

// Calculate pupil as mean of palpebral superius/inferius and (endo/exo)canthion.
// Does NOT ensure that the point is placed on the model surface!
FaceTools_EXPORT cv::Vec3f calcPupil( const Landmark::LandmarkSet& lmks, FaceLateral lat);

// Calculate face centre as the midpoint between the nasal root and subnasale.
FaceTools_EXPORT cv::Vec3f calcFaceCentre( const Landmark::LandmarkSet&);

// Return the point closest to v on the surface of the model.
FaceTools_EXPORT cv::Vec3f toSurface( const RFeatures::ObjModelKDTree*, const cv::Vec3f& v);

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
                                                                       const cv::Matx44d& T, std::unordered_map<int,int>* newVidxsToOld=nullptr);

// Flatten the source face to be in the XY plane, and set entries in provided map (if given) to key the source vertices
// from the new vertices in the returned (flattened) model.
FaceTools_EXPORT RFeatures::ObjModel::Ptr makeFlattened( const RFeatures::ObjModel* source, std::unordered_map<int,int>* newVidxsToOld=nullptr);

// Create and return an offscreen viewer for the given model with given square dimensions
// and camera at a distance of R along the Z axis and focused at the origin.
FaceTools_EXPORT RVTK::Viewer::Ptr makeOffscreenViewer( size_t sqdims, float R, const RFeatures::ObjModel*);

// Take and return a colour snapshot from the given viewer.
FaceTools_EXPORT cv::Mat_<cv::Vec3b> snapshot( RVTK::Viewer::Ptr v);

// Update exactly once all renderers referenced by all views of all models in the provided set.
FaceTools_EXPORT void updateRenderers( const FMS&);

}   // end namespace

#endif
