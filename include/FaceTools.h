/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include "FaceTypes.h"
#include "LndMrk/LandmarkSet.h"
#include "MiscFunctions.h"
#include <Viewer.h> // RVTK
#include <Orientation.h>    // RFeatures
#include <QtCharts/QChartView>

namespace FaceTools {

// Estimate the normal vector using an interative approach that evaluates
// normals along line segments measured over vertical line segments under the
// detected positions of the eyes. The left and right positions v0 and v1 should
// be given by the surface locations of the left and right pupils.
// After obtaining the normal vector, the up vector is calculated simply
// as the normalized vector vnorm.cross(v1-v0)
FaceTools_EXPORT void findNormal( const FM*,
                                  const cv::Vec3f& v0,    // Position of left eye
                                  const cv::Vec3f& v1,    // Position of right eye
                                  cv::Vec3f& vnorm);      // Output face norm

// Return the point closest to v on the surface of the model.
FaceTools_EXPORT cv::Vec3f toSurface( const FM*, const cv::Vec3f& v);

// Starting at the point on the surface closest to s, return the point on the surface closest to t.
FaceTools_EXPORT cv::Vec3f toTarget( const FM*, const cv::Vec3f& s, const cv::Vec3f& t);

FaceTools_EXPORT bool findPath( const FM*, const cv::Vec3f& p0, const cv::Vec3f& p1, std::list<cv::Vec3f>& pts);
FaceTools_EXPORT bool findStraightPath( const FM*, const cv::Vec3f& p0, const cv::Vec3f& p1, const cv::Vec3f& focVec, std::list<cv::Vec3f>& pts);
FaceTools_EXPORT bool findCurveFollowingPath( const FM*, const cv::Vec3f& p0, const cv::Vec3f& p1, std::list<cv::Vec3f>& pts);

// Find the point farthest off the straight line distance between p0 and p1 along the shortest contour over the mesh between these points.
// If parameter d is not null, on return it is set with the distance the returned point is orthogonally from the baseline vector p0->p1.
FaceTools_EXPORT cv::Vec3f findDeepestPoint( const FM*, const cv::Vec3f&, const cv::Vec3f&, double *d=nullptr);
FaceTools_EXPORT cv::Vec3f findDeepestPoint2( const FM*, const cv::Vec3f&, const cv::Vec3f&, double *d=nullptr);

// Calculate cropping radius for a face as G times the distance from the face centre to the point halfway between the eyes.
FaceTools_EXPORT double calcFaceCropRadius( const cv::Vec3f& faceCentre, const cv::Vec3f& leye, const cv::Vec3f& reye, double G);

// Create a vertices only ObjModel from the given row of points.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromVertices( const cv::Mat_<cv::Vec3f>& row);

// Create a vertices only ObjModel from the given subset of vertices.
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromSubset( const RFeatures::ObjModel&, const IntSet& vidxs);

// Given a source model and a subset of vertices (vidxs), create and return a new
// points only model consisting of the given vertices transformed using matrix T.
// On return, if newVidxsToOld is not null, it is set with vertex mappings of the
// new (returned) object to the given vertex indices on the source model (vidxs).
FaceTools_EXPORT RFeatures::ObjModel::Ptr createFromTransformedSubset( const RFeatures::ObjModel& source, const IntSet& vidxs,
                                                                       const cv::Matx44d& T, std::unordered_map<int,int>* newVidxsToOld=nullptr);

// Flatten the source face to be in the XY plane, and set entries in provided map (if given) to key the source vertices
// from the new vertices in the returned (flattened) model.
FaceTools_EXPORT RFeatures::ObjModel::Ptr makeFlattened( const RFeatures::ObjModel& source, std::unordered_map<int,int>* newVidxsToOld=nullptr);

// Update exactly once all renderers referenced by all views of all models in the provided set.
FaceTools_EXPORT void updateRenderers( const FMS&);

FaceTools_EXPORT cv::Mat_<cv::Vec3b> makeThumbnail( const FM*, const cv::Size& dims, float d);

// Return a colour giving best contrast with the parameter colour.
FaceTools_EXPORT QColor chooseContrasting( const QColor&);

}   // end namespace

#endif
