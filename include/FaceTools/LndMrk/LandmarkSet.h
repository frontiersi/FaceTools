/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_LANDMARK_LANDMARK_SET_H
#define FACE_TOOLS_LANDMARK_LANDMARK_SET_H

#include "Landmark.h"
#include "LandmarksManager.h"
#include <r3d/Mesh.h>
#include <r3d/Bounds.h>
#include <boost/property_tree/ptree.hpp>

namespace FaceTools { namespace Landmark {

using PTree = boost::property_tree::ptree;

class FaceTools_EXPORT LandmarkSet
{
public:
    LandmarkSet(){}
    LandmarkSet( const LandmarkSet&) = default;
    LandmarkSet& operator=( const LandmarkSet&) = default;

    /**
     * Create a set of landmarks from a set of landmark sets!
     * The client should call moveToSurface passing a model on the returned
     * landmarks set if desirable for the points be coincident with a surface.
     */
    LandmarkSet( const std::unordered_set<const LandmarkSet*>&);

    // Add/change position of landmark with given id on given lateral.
    // If landmark is not bilateral, the FaceSide argument may be ignored.
    // Returns true iff landmark with given id on the given lateral exists.
    bool set( int id, const Vec3f&, FaceSide=MID);
    bool set( const QString& code, const Vec3f&, FaceSide=MID);

    // Return the full set of landmarks for one lateral.
    const std::unordered_map<int, Vec3f>& lateral( FaceSide) const;

    // Return the position of the landmark for the given lateral.
    // The FACE_LATERAL_MEDIAL constant does not need to be specified if the
    // landmark is medial. Note though that only the given lateral is checked.
    const Vec3f& pos( int id, FaceSide=MID) const;

    // Alternative usage takes a landmark code.
    const Vec3f& pos( const QString& lmcode, FaceSide=MID) const;

    // Another alternative.
    const Vec3f& pos( const SpecificLandmark&) const;

    // Get a point in space from a list of (possibly weighted) landmarks.
    Vec3f toPoint( const LmkList &ll, const Mat4f& T, const Mat4f& iT) const;

    // Create and return this set of landmarks as a mesh. The advantage of this
    // is that the landmarks can be treated as their own model and every landmark
    // has a distinct (ordered) vertex ID meaning that the full set of landmarks
    // can quickly be turned into a matrix (Mesh::vertices2Matrix).
    r3d::Mesh::Ptr toMesh() const;

    // Convenience function to return the interpupil difference vector (from left to right eye),
    // or the zero vector if landmarks not available.
    Vec3f eyeVec() const;

    // Returns point midway between left and right eyes (NOT ON SURFACE!)
    Vec3f midEyePos() const;

    // Make bounds that contain this set of landmarks. Pass in the current
    // transform and inverse transform matrices.
    r3d::Bounds::Ptr makeBounds( const Mat4f &T, const Mat4f &iT) const;

    // Returns squared radius from medialMean() to the most distant landmark from it.
    float sqRadius() const;

    // Returns the mean of the landmarks in the designated medial set.
    Vec3f medialMean() const;

    // Swap the left and right laterals.
    void swapLaterals();

    void moveToSurface( const FM*);

    void transform( const Mat4f&);

    // Returns the position of a visible landmark closest to the given position
    // but only if it's within the given squared distance.
    Vec3f snapTo( const Vec3f&, float snapSqDist) const;

    // Get the ids of landmarks in this set.
    const IntSet& ids() const { return _ids;}

    // Set membership tests
    bool has( int id) const { return _ids.count(id) > 0;}
    bool has( int id, FaceSide) const;   // Landmark with given id present for the given lateral?
    bool has( const SpecificLandmark&) const;

    // Set size tests
    size_t size() const { return _lmksL.size() + _lmksM.size() + _lmksR.size();}
    bool empty() const { return _ids.empty();}

    // Returns false if content could not be read in.
    bool read( const PTree&);

    // Write these landmark data out.
    void write( PTree&) const;

private:
    IntSet _ids;

    using LDMRKS = std::unordered_map<int, Vec3f>;
    LDMRKS _lmksL;  // Subject's left side
    LDMRKS _lmksM;  // Middle
    LDMRKS _lmksR;  // Subject's right side

    LDMRKS& _lateral( FaceSide);
    bool _readLateral( const PTree&, FaceSide);
    Vec3f _quarter0() const;
    Vec3f _quarter1() const;
    Vec3f _quarter2() const;
    Vec3f _quarter3() const;

    /*
    mutable Mat4f _algn;    // Alignment matrix calculated as needed
    mutable Mat4f _ialgn;   // Inverse of the alignment matrix calculated as needed
    void _clearAlignment() const;
    */
};  // end class

}}   // end namespace

#endif
