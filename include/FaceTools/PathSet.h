/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

namespace FaceTools {

class FaceTools_EXPORT PathSet
{
public:
    PathSet();
    ~PathSet();
    PathSet( const PathSet&) = default;
    PathSet& operator=( const PathSet&) = default;

    // Translates all path endpoints to be incident with surface and recalculates path vertices.
    void update( const FM*);

    // Create a new path with first handle at given position returning its ID.
    int addPath( const Vec3f&);

    // Move in a new path setting a new ID (to prevent overwriting).
    int addPath( Path&&);

    // Remove the path with given ID returning true on success.
    bool removePath( int pathId);

    void reset();

    // Rename the path with given ID returning true on success.
    bool renamePath( int pathId, const std::string&);

    // Return the path with given ID.
    const Path& path( int pathId) const;

    Path& path( int pathId);

    QString name( int pathId) const;

    const IntSet& ids() const { return _ids;}

    // Set membership tests
    inline bool empty() const { return size() == 0;}
    inline size_t size() const { return _paths.size();}
    inline bool has( int pathId) const { return _ids.count(pathId) > 0;}

    // Add T onto the paths, and use the total inverse rotation matrix
    // (if given) to update measurements (specifically, angles).
    void transform( const Mat4f &T, const Mat3f *iR=nullptr);

    void write( PTree&, bool withExtras) const;
    bool read( const PTree&);

private:
    int _sid;                               // Set's next path ID
    IntSet _ids;                            // The IDs themselves
    std::unordered_map<int, Path> _paths;   // IDs that map to paths
    int _setPath( Path&&);
};  // end class

}   // end namespace

#endif
