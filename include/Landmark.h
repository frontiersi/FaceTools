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

#ifndef FACE_TOOLS_LANDMARK_H
#define FACE_TOOLS_LANDMARK_H

#ifdef _WIN32
#pragma warning( disable : 4251)
#endif

#include "FaceTools_Export.h"
#include "Landmarks.h"
#include <Orientation.h>    // RFeatures

namespace FaceTools {

struct FaceTools_EXPORT Landmark
{
    Landmark() : id(-1) {}   // Requires default instantiation (immediately overwrite with name/pos constructed Landmark)
    Landmark( int i, const std::string& nm, const cv::Vec3f& v, bool vis=true, bool mov=true, bool del=true)
        : id(i), name(nm), pos(v), visible(vis), movable(mov), deletable(del) {}

    int id;
    std::string name;
    cv::Vec3f pos;
    bool visible;
    bool movable;
    bool deletable;
};  // end struct


FaceTools_EXPORT PTree& operator<<( PTree&, const Landmark&);   // Write out the given landmark to the property_tree

}   // end namespace

#endif
