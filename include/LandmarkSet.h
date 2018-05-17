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

#ifndef FACE_TOOLS_LANDMARK_SET_H
#define FACE_TOOLS_LANDMARK_SET_H

#include "Landmark.h"
#include <ObjModelKDTree.h>
#include <unordered_map>
#include <unordered_set>

namespace FaceTools {

class FaceTools_EXPORT LandmarkSet
{
public:
    LandmarkSet();
    LandmarkSet( const LandmarkSet&);
    LandmarkSet& operator=( const LandmarkSet&);
    virtual ~LandmarkSet(){}

    // Set a landmark (new or extant) to position v.
    int set( const std::string& name, const cv::Vec3f& v); // New or existing.
    bool set( int id, const cv::Vec3f& v);                 // Existing only (returns true on success).

    // Return the position of the landmark only (convenience function).
    const cv::Vec3f& pos( const std::string& name) const;
    const cv::Vec3f& pos( int id) const;

    // Returns landmark data for getting/setting or NULL if no landmark with that name/id exists.
    const Landmark* get( const std::string& name) const;
    Landmark* get( const std::string& name);
    const Landmark* get( int id) const;
    Landmark* get( int id);

    // Returns true if landmark existed and was removed.
    bool erase( const std::string& name);
    bool erase( int id);
    
    // Returns true if landmark with given ID exists and new name doesn't overwrite an existing landmark.
    bool changeName( int id, const std::string& newname);

    // Add translation vector v to specified landmark (returns false if landmark not present).
    bool translate( const std::string& name, const cv::Vec3f&);
    bool translate( int id, const cv::Vec3f&);
    void translate( const cv::Vec3f&);     // Add translation vector v to all landmarks
    void transform( const cv::Matx44d&);   // Transform landmarks according to the given matrix.

    // Get landmark names and IDs
    const std::unordered_set<std::string>& names() const { return _names;}
    const std::unordered_set<int>& ids() const { return _ids;}

    // Set membership tests
    bool empty() const { return count() == 0;}
    size_t count() const { return _landmarks.size();}
    bool has( int id) const { return _ids.count(id) > 0;}
    bool has( const std::string& name) const { return _names.count(name) > 0;}

private:
    int _sid;   // Incrementing IDs for newly created Landmarks.
    std::unordered_map<std::string, Landmark> _landmarks;  // Landmark names to Landmarks
    std::unordered_map<int, std::string> _landmarkIDs;     // Landmark IDs to names
    std::unordered_set<std::string> _names;
    std::unordered_set<int> _ids;
};  // end class


// Translate each landmark in the set to be incident with the closest part of the given object's surface.
// Returns the mean positional difference between the old and new landmark positions.
FaceTools_EXPORT double translateLandmarksToSurface( const RFeatures::ObjModelKDTree&, LandmarkSet&);


FaceTools_EXPORT PTree& operator<<( PTree&, const LandmarkSet&);        // Write out the set of landmarks
FaceTools_EXPORT const PTree& operator>>( const PTree&, LandmarkSet&);  // Read in the set of landmarks

}   // end namespace

#endif
