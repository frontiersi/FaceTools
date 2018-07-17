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

#ifndef FACE_TOOLS_PATH_SET_H
#define FACE_TOOLS_PATH_SET_H

#include "Path.h"
#include <unordered_map>
#include <memory>

namespace FaceTools {

class PathSet;

FaceTools_EXPORT PTree& operator<<( PTree&, const PathSet&);        // Write out the set of paths
FaceTools_EXPORT const PTree& operator>>( const PTree&, PathSet&); // Read in the set of paths


class FaceTools_EXPORT PathSet
{
public:
    typedef std::shared_ptr<PathSet> Ptr;
    static Ptr create();

    // Translates path endpoints to be incident with surface and recalculates the paths.
    void recalculate( RFeatures::ObjModelKDTree::Ptr);

    // Create a new path with first handle at given position returning its ID.
    int addPath( const cv::Vec3f&);

    // Remove the path with given ID returning true on success.
    bool removePath( int pathId);

    // Return the path with given ID or null if not found.
    Path* path( int pathId);
    const Path* path( int pathId) const;

    const std::unordered_set<int>& ids() const { return _ids;}

    // Set membership tests
    bool empty() const { return count() == 0;}
    size_t count() const { return _paths.size();}
    bool has( int pathId) const { return _ids.count(pathId) > 0;}

    void transform( const cv::Matx44d&);

private:
    int _sid;
    std::unordered_map<int, Path> _paths;   // IDs that map to paths
    std::unordered_set<int> _ids;           // The IDs themselves
    int setPath( const Path&);
    friend PTree& FaceTools::operator<<( PTree&, const PathSet&);
    friend const PTree& FaceTools::operator>>( const PTree&, PathSet&);

    PathSet();
    ~PathSet();
    PathSet( const PathSet&);           // No copy
    void operator=( const PathSet&);    // No copy
};  // end class


}   // end namespace

#endif
