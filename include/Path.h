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

#ifndef FACE_TOOLS_PATH_H
#define FACE_TOOLS_PATH_H

#ifdef _WIN32
#pragma warning( disable : 4251)
#endif

#include "FaceTools_Export.h"
#include <ObjModelKDTree.h>
#include <opencv2/opencv.hpp>
#include <boost/property_tree/ptree.hpp>
typedef boost::property_tree::ptree PTree;

namespace FaceTools {

struct FaceTools_EXPORT Path
{
    Path();
    Path( int id, const cv::Vec3f& v0);

    // Set path endpoints on the model surface and recalculate
    // path and both path lengths using the given KD-tree.
    // If no path could be found, return false.
    bool recalculate( const RFeatures::ObjModelKDTree*);

    int id;
    double elen;                // Cached straight line distance (l2-norm between path end-points)
    double psum;                // Cached path sum
    std::list<cv::Vec3f> vtxs;  // The path vertices 
    std::string name;           // Name of this path (if given)
};  // end class


// Write out path data including lengths but ONLY the path end-point vertices!
FaceTools_EXPORT PTree& operator<<( PTree&, const Path&);

// After reading in, path vertices will need calculating via a call to recalculate.
FaceTools_EXPORT const PTree& operator>>( const PTree&, Path&);


}   // end namespace

#endif
