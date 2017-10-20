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

#ifndef FACE_TOOLS_FACE_ORIENTER_H
#define FACE_TOOLS_FACE_ORIENTER_H

#include "FaceTools_Export.h"
#include <ObjModelCurvatureMap.h>   // RFeatures
#include <ObjModelKDTree.h>         // RFeatures

namespace FaceTools
{

class FaceTools_EXPORT FaceOrienter
{
public:
    // Static functions to simply calculate up and normal vectors given positions of the nose bridge
    // and the left and right eyes. Function calcUp returns the up vector for the given position vectors.
    static cv::Vec3d calcUp( const cv::Vec3f& nbridge, const cv::Vec3f& leftEye, const cv::Vec3f& rightEye);

    // Calculate the normal using the up vector returned from calcUp and the left and right eye locations.
    static cv::Vec3d calcNormal( const cv::Vec3f& upv, const cv::Vec3f& leftEye, const cv::Vec3f& rightEye);

    // Will construct the KD-tree if not provided.
    FaceOrienter( const RFeatures::ObjModelCurvatureMap::Ptr,
                  const RFeatures::ObjModelKDTree::Ptr=RFeatures::ObjModelKDTree::Ptr());

    // Estimate the up and normal vectors using an interative approach that evaluates
    // normals along line segments measured over portions of the face. The left and right
    // positions v0 and v1 should be given by the surface vertices at the left and right pupils.
    bool operator()( const cv::Vec3f& v0, const cv::Vec3f& v1, cv::Vec3d& vnorm, cv::Vec3d& vup) const;

private:
    const RFeatures::ObjModelCurvatureMap::Ptr _curvMap;
    const RFeatures::ObjModelKDTree::Ptr _kdtree;
    cv::Vec3d findNormalEstimate( const cv::Vec3d&, int, int) const;
    cv::Vec3d calcMeanNormalBetweenPoints( int, int) const;
};  // end class

}   // end namespace

#endif
