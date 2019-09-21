/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <ObjModelKDTree.h>
#include <Orientation.h>
#include <boost/property_tree/ptree.hpp>
using PTree = boost::property_tree::ptree;

namespace FaceTools { namespace Landmark {

class FaceTools_EXPORT LandmarkSet
{
public:
    using Ptr = std::shared_ptr<LandmarkSet>;
    using CPtr = std::shared_ptr<const LandmarkSet>;

    static Ptr create();

    /**
     * Create and return a mean set of landmarks from the provided set.
     * The client should call moveToSurface passing a model on the returned
     * landmarks set if is desirable for the points be coincident with a surface.
     */
    static Ptr createMean( const std::unordered_set<const LandmarkSet*>&);

    Ptr deepCopy() const;

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

    // Return the full set of landmarks for one lateral. These landmarks are untransformed.
    const std::unordered_map<int, cv::Vec3f>& lateral( FaceLateral) const;

    // Return the position of the landmark for the given lateral.
    // The FACE_LATERAL_MEDIAL constant does not need to be specified if the
    // landmark is medial. Note though that only the given lateral is checked.
    cv::Vec3f pos( int id, FaceLateral=FACE_LATERAL_MEDIAL) const;

    // Return the untransformed position for the given landmark.
    const cv::Vec3f& upos( int id, FaceLateral=FACE_LATERAL_MEDIAL) const;

    // Alternative usage takes a landmark code.
    cv::Vec3f pos( const QString& lmcode, FaceLateral=FACE_LATERAL_MEDIAL) const;

    // Return the untransformed position for the given landmark.
    const cv::Vec3f& upos( const QString& lmcode, FaceLateral=FACE_LATERAL_MEDIAL) const;

    // Another alternative.
    cv::Vec3f pos( const SpecificLandmark&) const;

    // Return the position of some random medial landmark if one exists, else null.
    cv::Vec3f posSomeMedial() const;

    // Convenience function to return the interpupil difference vector (from left to right eye),
    // or the zero vector if landmarks not available.
    cv::Vec3f eyeVec() const;

    // Calculate the right vector from all pairs of lateral landmarks.
    cv::Vec3f rightVec() const;

    // Calculate the up vector as l2norm( superiorMean - inferiorMean).
    cv::Vec3f upVec() const;

    // Calculate the normal vector as rightVec X upVec.
    cv::Vec3f normVec() const;

    // Calculate and return orientation.
    RFeatures::Orientation orientation() const;

    // Get the mean position of the landmarks in the superior (top face) group.
    // The superior group consists of the 10 landmarks:
    // Mid-supraorbital (LR)
    // Glabella
    // Exocanthion (LR)
    // Endocanthion (LR)
    // Nasion
    // Sellion
    // Mid-nasal Dorsum
    cv::Vec3f superiorMean() const;

    // Get the mean position of the landmarks in the inferior (bottom face) group.
    // The inferior landmark group consists of the 10 landmarks:
    // Alar Curvature Point (LR)
    // Christa Philtri (LR)
    // Cheilion (LR)
    // Labiale Superius
    // Labiale Inferius
    // Stomion (mean of Superius and Inferius)
    // Sublabiale
    cv::Vec3f inferiorMean() const;

    cv::Vec3f fullMean() const; // Simply (superiorMean + inferiorMean)/2

    // Add translation vector v to specified landmark (returns false if landmark not present).
    //bool translate( int id, FaceLateral, const cv::Vec3f&);
    void swapLaterals();                   // Swap the left and right laterals.

    void moveToSurface( const FM*);

    void addTransformMatrix( const cv::Matx44d&);
    void fixTransformMatrix();
    const cv::Matx44d& transformMatrix() const { return _tmat;}

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
    cv::Matx44d _tmat;  // Transform matrix
    cv::Matx44d _imat;  // Inverse transform matrix
    using LDMRKS = std::unordered_map<int, cv::Vec3f>;
    LDMRKS _lmksL;  // Left lateral
    LDMRKS _lmksM;  // Medial (none)
    LDMRKS _lmksR;  // Right lateral

    std::unordered_map<int, cv::Vec3f>& lateral( FaceLateral);
    bool readLateral( const PTree&, FaceLateral);

    LandmarkSet();
    ~LandmarkSet(){}
    LandmarkSet( const LandmarkSet&) = default;
    LandmarkSet& operator=( const LandmarkSet&) = default;
};  // end class

}}   // end namespace

#endif
