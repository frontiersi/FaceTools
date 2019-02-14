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

#ifndef FACE_TOOLS_LANDMARK_LANDMARK_SET_H
#define FACE_TOOLS_LANDMARK_LANDMARK_SET_H

#include <Landmark.h>
#include <LandmarksManager.h>
#include <ObjModelKDTree.h>
#include <boost/property_tree/ptree.hpp>
using PTree = boost::property_tree::ptree;

namespace FaceTools { namespace Landmark {

class FaceTools_EXPORT LandmarkSet
{
public:
    using Ptr = std::shared_ptr<LandmarkSet>;
    static Ptr create();

    // Add/change position of landmark with given id on given lateral.
    // If landmark is not bilateral, the FaceLateral argument may be ignored.
    // Returns true iff landmark with given id on the given lateral exists.
    bool set( int id, const cv::Vec3f&, FaceLateral=FACE_LATERAL_MEDIAL);
    bool set( const QString& code, const cv::Vec3f&, FaceLateral=FACE_LATERAL_MEDIAL);

    // Deletes landmark(s) and returns true if landmark with given id exists.
    // In the case of bilateral landmarks, both landmarks are deleted.
    bool erase( int id);

    // Remove all landmarks from the set.
    void clear();

    // Return the position of the landmark for the given lateral.
    // The FACE_LATERAL_MEDIAL constant does not need to be specified if the
    // landmark is medial. Note though that only the given lateral is checked.
    // Returns null if the landmark is not present (on the given lateral).
    const cv::Vec3f* pos( int id, FaceLateral=FACE_LATERAL_MEDIAL) const;

    // Alternative usage takes a landmark code.
    const cv::Vec3f* pos( const QString& lmcode, FaceLateral=FACE_LATERAL_MEDIAL) const;

    // Another alternative.
    const cv::Vec3f* pos( const SpecificLandmark&) const;

    // Convenience function to return the interpupil difference vector (from left to right eye),
    // or the zero vector if landmarks not available.
    cv::Vec3f eyeVec() const;

    // Add translation vector v to specified landmark (returns false if landmark not present).
    bool translate( int id, FaceLateral, const cv::Vec3f&);
    void translate( const cv::Vec3f&);     // Add translation vector v to all landmarks
    void transform( const cv::Matx44d&);   // Transform landmarks according to the given matrix.

    void moveToSurface( const RFeatures::ObjModelKDTree*);

    // Get the names/codes/ids of landmarks in this set.
    const QStringSet& names() const { return _names;}
    const QStringSet& codes() const { return _codes;}
    const IntSet& ids() const { return _ids;}

    // Set membership tests
    bool has( int id) const { return _ids.count(id) > 0;}
    bool has( int id, FaceLateral) const;   // Landmark with given id present for the given lateral?
    bool has( const SpecificLandmark&) const;
    bool hasName( const QString& n) const { return _names.count(n) > 0;}
    bool hasCode( const QString& c) const { return _codes.count(c) > 0;}

    // Set size tests
    size_t size() const { return _lmksL.size() + _lmksM.size() + _lmksR.size();}
    bool empty() const { return size() == 0;}

    // Returns false if content could not be read in.
    bool read( const PTree&);

    // Write these landmark data out.
    void write( PTree&) const;

private:
    IntSet _ids;
    QStringSet _names, _codes;
    using LDMRKS = std::unordered_map<int, cv::Vec3f>;
    LDMRKS _lmksL;  // Left lateral
    LDMRKS _lmksM;  // Medial (none)
    LDMRKS _lmksR;  // Right lateral

    const std::unordered_map<int, cv::Vec3f>& lateral( FaceLateral) const;
    std::unordered_map<int, cv::Vec3f>& lateral( FaceLateral);

    LandmarkSet();
    ~LandmarkSet(){}
    LandmarkSet( const LandmarkSet&) = delete;
    void operator=( const LandmarkSet&) = delete;
    bool readLateral( const PTree&, FaceLateral);
};  // end class

}}   // end namespace

#endif
