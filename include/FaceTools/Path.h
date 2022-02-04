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

#ifndef FACE_TOOLS_PATH_H
#define FACE_TOOLS_PATH_H

#ifdef _WIN32
#pragma warning( disable : 4251)
#endif

#include "FaceTypes.h"
#include <boost/property_tree/ptree.hpp>
using PTree = boost::property_tree::ptree;

namespace FaceTools {

class FaceTools_EXPORT Path
{
public:
    enum PathType
    {
        CURVE_FOLLOWING_0,
        CURVE_FOLLOWING_1,
        STRAIGHT_CURVE,
        ORIENTED_CURVE
    };  // end enum

    // Set the path type to use (defaults to ORIENTED_CURVE)
    static void setPathType( PathType);

    Path();
    Path( const Path&) = default;
    Path( Path&&) = default;
    Path& operator=( const Path&) = default;
    Path( int id, const Vec3f& v0);

    // Return a copy of this path but with all vertices barycentrically mapped
    // from the source to the given destination . Function updateMeasures is
    // called on the created path before being returned.
    Path mapSrcToDst( const FM *src, const FM *dst) const;

    void setId( int id) { _id = id;}
    inline int id() const { return _id;}

    void setName( const std::string &nm) { _name = nm;}
    inline const std::string& name() const { return _name;}

    // Calls setHandle0, setHandle1, setDepthHandle based on hid.
    void setHandle( int hid, const Vec3f&);

    // Returns position of path handles based on hid.
    // (0 == handle0, 1 == handle1, 2 == depthHandle)
    const Vec3f& handle( int hid) const;

    void setHandle0( const Vec3f&);
    inline const Vec3f& handle0() const { return _vtxs.front();}
    void setHandle1( const Vec3f&);
    inline const Vec3f& handle1() const { return _vtxs.back();}
    void setDepthHandle( const Vec3f&);
    inline const Vec3f& depthHandle() const { return _dhan;}

    // Return the difference vector between the path endpoints.
    Vec3f deltaVector() const { return handle1() - handle0();}

    void setOrientation( const Vec3f&);
    inline const Vec3f& orientation() const { return _orient;}

    // Set path endpoints on the model surface and recalculate path and both path
    // lengths using the given model. If no path could be found, return false.
    // Remember to call updateMeasures after calling this function and setting the depth handle!
    bool updatePath( const FM*);

    // After calling updatePath, or setting the depth handle, call this to update measurements.
    // The inverse rotation matrix is required to ensure angles are projected into the facial
    // planes irrespective of the model's current transform (rotation).
    void updateMeasures( const Mat3f& inverseRotation);

    inline bool validPath() const { return _validPath;}
    inline float euclideanDistance() const { return _elen;}
    inline float surfaceDistance() const { return _slen;}
    inline float surface2EuclideanRatio() const { return _elen > 0.0f ? _slen / _elen : 1.0f;}
    inline const Vec3f& depthSurfPoint() const { return _dsurf;} // Point on surface at depth point
    inline const Vec3f& depthLinePoint() const { return _dline;} // Depth handle projected to line
    inline float crossSectionalArea() const { return _area;}
    inline float angle() const { return _angle;}            // Angle in degrees
    inline float angleSagittal() const { return _angleS;}   // Angle in degrees
    inline float angleTransverse() const { return _angleT;} // Angle in degrees
    inline float angleCoronal() const { return _angleC;}    // Angle in degrees
    inline float depth() const { return _depth;}

    // Always at least of size 2.
    inline const std::list<Vec3f>& pathVertices() const { return _vtxs;}

    void transform( const Mat4f&);

    void write( PTree&, bool withFullPath) const;

    // After reading in, path vertices will need calculating via a call to updatePath.
    void read( const PTree&);

private:
    static PathType s_pathType;
    int _id;
    std::string _name;      // Name of this path
    bool _validPath;        // True if path lies on surface
    std::list<Vec3f> _vtxs; // The path vertices
    float _elen;            // Euclidean distance
    float _slen;            // Surface distance
    float _area;            // Cross sectional area
    float _depth;           // Straight line magnitude from surface to depth handle
    float _angle;           // Angle in degrees
    float _angleS;          // Angle projected into sagittal
    float _angleT;          // Angle projected into transverse
    float _angleC;          // Angle projected into coronal (frontal)
    Vec3f _dhan;            // Depth handle position
    Vec3f _dsurf;           // Point on path orthogonal to handles line at depth handle position
    Vec3f _dline;           // Depth handle projected along direct line between handles
    Vec3f _orient;          // View plane orientation of the path
    void _writeFullPath( PTree&) const;
    void _calcAngles( const Mat3f&);
};  // end class

}   // end namespace

#endif
