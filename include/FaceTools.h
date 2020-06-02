/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include "FaceTools/FaceTypes.h"
#include <r3d/KDTree.h>

namespace FaceTools {

// Estimate the normal vector using an interative approach that evaluates
// normals along line segments measured over vertical line segments under the
// detected positions of the eyes. The left and right positions v0 and v1 should
// be given by the surface locations of the left and right pupils.
// After obtaining the normal vector, the up vector is calculated simply
// as the normalized vector vnorm.cross(v1-v0)
FaceTools_EXPORT Vec3f findNormal( const r3d::KDTree&,
                                   const Vec3f &v0,     // Position of left eye
                                   const Vec3f &v1,     // Position of right eye
                                   const Vec3f &vnorm); // Output face norm

// Return the point closest to v on the surface of the model.
FaceTools_EXPORT Vec3f toSurface( const r3d::KDTree&, const Vec3f& v);

// Starting at the point on the surface closest to s, return the point on the surface closest to t.
FaceTools_EXPORT Vec3f toTarget( const r3d::KDTree&, const Vec3f& s, const Vec3f& t);

FaceTools_EXPORT bool findPath( const r3d::KDTree&, const Vec3f& p0, const Vec3f& p1, std::list<Vec3f>& pts);
FaceTools_EXPORT bool findStraightPath( const r3d::KDTree&, const Vec3f& p0, const Vec3f& p1, std::list<Vec3f>& pts);
FaceTools_EXPORT bool findCurveFollowingPath( const r3d::KDTree&, const Vec3f& p0, const Vec3f& p1, std::list<Vec3f>& pts);
FaceTools_EXPORT bool findOrientedPath( const r3d::KDTree&, const Vec3f& p0, const Vec3f& p1, const Vec3f& u, std::list<Vec3f>&);

// Find the hill or valley point between p0 and p1 along the straight contour between these points.
FaceTools_EXPORT Vec3f findHighOrLowPoint( const r3d::KDTree&, const Vec3f&, const Vec3f&);

// Calculate cropping radius for a face as G times the distance from the face centre to the point halfway between the eyes.
FaceTools_EXPORT float calcFaceCropRadius( const Vec3f& faceCentre, const Vec3f& leye, const Vec3f& reye, float G);

// Update exactly once all renderers referenced by all views of all models in the provided set.
FaceTools_EXPORT void updateRenderers( const FMS&);

FaceTools_EXPORT cv::Mat_<cv::Vec3b> makeThumbnail( const FM*, const cv::Size& dims, float d);

// Return a colour giving best contrast with the parameter colour.
FaceTools_EXPORT QColor chooseContrasting( const QColor&);

}   // end namespace

#endif
