/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_FACE_MODEL_H
#define FACE_TOOLS_FACE_MODEL_H

#include "FaceAssessment.h"
#include "FaceViewSet.h"
#include <QReadWriteLock>
#include <QDate>
#include <r3d.h>

namespace FaceTools {

class FaceTools_EXPORT FaceModel
{
public:
    explicit FaceModel( r3d::Mesh::Ptr);
    FaceModel();

    void lockForWrite();        // Lock before making write changes to this FaceModel.
    void lockForRead() const;   // Lock before reading this FaceModel's state.
    void unlock() const;        // Call after done with read or write locks.

    /**
     * Update with new mesh - must be different from the existing mesh!
     * If updateConnectivity is true, manifolds are reparsed which is an expensive operation.
     * If settleLandmarks is true, landmarks and other items that rest on the surface are reseated.
     * This should normally be true unless setting the mesh for the first time after reading
     * in landmark/path positions. If maxManifolds > 0, this will override the default number
     * of manifolds to set (MAX_MANIFOLDS).
     * View actors should be rebuilt after calling this function.
     */
    void update( r3d::Mesh::Ptr, bool updateConnectivity, bool settleLandmarks, int maxManifolds=-1);

    /**
     * Convenience function for fixing the transform matrix and updating the internal mesh is changed.
     * Treat as update; view actors should be rebuilt after calling this function.
     */
    void fixTransformMatrix();

    /**
     * For making rigid affine changes to the data that can be expressed using a matrix.
     * Transform the mesh, search tree, landmarks and paths using the given matrix
     * as well as all FaceViews associated with this model. Note that only paths are
     * updated directly; the mesh, bounds and landmarks are set with
     * matrices to transform their vertices.
     */
    void addTransformMatrix( const Mat4f&);

    /**
     * Call to remake bounds when setting landmarks for first time.
     */
    void remakeBounds();

    /**
     * Return the model's current transform matrix (or its inverse).
     */
    Mat4f transformMatrix() const;
    Mat4f inverseTransformMatrix() const;

    /**
     * Return the centre of the front panel of the model's bounding box.
     */
    Vec3f centreFront() const;

    /**
     * Returns true iff the current tranform matrix is the unit matrix.
     */
    bool isAligned() const;

    const r3d::Mesh& mesh() const { return *_mesh;}
    const r3d::KDTree& kdtree() const { return *_kdtree;}
    const r3d::Manifolds& manifolds() const { return *_manifolds;}
    bool hasTexture() const { return _mesh->hasMaterials();}

    /**
     * Returns the ID of the manifold holding the face or -1 if landmarks not yet set.
     */
    int faceManifoldId() const;

    /**
     * Returns full model bounds at entry zero, and corresponding manifold bounds at higher indices.
     * Without landmarks, the bounds at entry zero encompass the model. With landmarks, the bounds at
     * entry zero are defined according to the alignment matrix determined by the landmarks.
     */
    const std::vector<r3d::Bounds::Ptr>& bounds() const { return _bnds;}

    /**
     * Set/get the mask this model is registered against.
     * On setting, texture coordinate information from the internal mesh is copied over.
     * Note that the mask is always initially set with its transform as the identity
     * matrix even if this model has a non-identity transform set.
     */
    void setMask( r3d::Mesh::Ptr, bool copyInTexture=false);
    const r3d::Mesh &mask() const { return *_mask;}
    const r3d::KDTree& maskKDTree() const { return *_mkdtree;}

    /**
     * Set/get the filename and hash associated with the mask.
     */
    void setMaskHash( size_t);
    size_t maskHash() const { return _maskHash;}

    /**
     * Returns true iff this model has an associated mask set.
     */
    bool hasMask() const { return _mask != nullptr;}

    /**
     * Set the given assessment in this model, overwriting what exists at the corresponding
     * index and potentially setting the current assessment too if the given assessment has
     * the same id.
     */
    void setAssessment( FaceAssessment::Ptr);

    /**
     * Erase the assessment with given ID. Must always have at least one assessment!
     * If the erased assessment is the current assessment, the current assessment is
     * set to a random assessment.
     */
    void eraseAssessment( int);

    /**
     * Set the current assessment using its id (must exist).
     */
    void setCurrentAssessment( int);

    /**
     * Return the current assessment.
     */
    FaceAssessment::Ptr currentAssessment() { return _cass;}
    FaceAssessment::CPtr currentAssessment() const { return _cass;}
    FaceAssessment::CPtr cassessment() const { return currentAssessment();}  // For Lua binding

    /**
     * Return the assessment with the given id. If id is left
     * as -1, these functions just return the current assessment.
     */
    FaceAssessment::CPtr assessment( int id=-1) const;
    FaceAssessment::Ptr assessment( int id=-1);

    /**
     * Return all assessment ids.
     */
    IntSet assessmentIds() const;

    /**
     * Return the number of assessments; should always be >= 1.
     */
    int assessmentsCount() const { return _ass.size();}

    /**
     * Returns true iff the current assessment has landmarks set.
     */
    bool hasLandmarks() const;

    /**
     * Set landmarks on the current assessment.
     */
    void setLandmarks( const Landmark::LandmarkSet&);

    /**
     * Convenience function to return const landmarks from the current assessment.
     */
    const Landmark::LandmarkSet& currentLandmarks() const;

    /**
     * Set the position of the landmark with given ID on the current assessment.
     * Position should be as found on the currently transformed model.
     */
    void setLandmarkPosition( int, FaceLateral, const Vec3f&);

    /**
     * Swap the landmark laterals for ALL assessments.
     */
    void swapLandmarkLaterals();

    /**
     * Make landmarks and paths update their positions to be incident with the surface. Makes metadata unsaved.
     */
    void moveToSurface();

    /**
     * Create and return new landmarks set being average over all individual assessment
     * landmark sets. Note that landmark positions are never transformed.
     */
    Landmark::LandmarkSet makeMeanLandmarksSet() const;

    /**
     * Convenience function to return const paths from the current assessment.
     */
    const PathSet& currentPaths() const;

    /**
     * Add a path with its initial (transformed) position and both handles are set to.
     */
    int addPath( const Vec3f&);

    void removePath( int pid);
    void renamePath( int pid, const QString&);

    // Set/get source of data.
    void setSource( const QString&);
    const QString& source() const { return _source;}

    // Set/get study ID
    void setStudyId( const QString&);
    const QString& studyId() const { return _studyId;}

    // Get age of individual at date of image capture.
    float age() const { return float( dateOfBirth().daysTo( captureDate())) / 365.25f;}

    // Set/get DoB of individual.
    void setDateOfBirth( const QDate&);
    const QDate& dateOfBirth() const { return _dob;}

    // Set/get sex of individual.
    void setSex( int8_t);
    int8_t sex() const { return _sex;}

    /**
     * Set/get ethnicity of individual's mother.
     */
    void setMaternalEthnicity( int);
    int maternalEthnicity() const { return _methnicity;}

    /**
     * Set/get ethnicity of individual's father.
     */
    void setPaternalEthnicity( int);
    int paternalEthnicity() const { return _pethnicity;}

    // Set/get capture date of image.
    void setCaptureDate( const QDate&);
    const QDate& captureDate() const { return _cdate;}

    // Set/get if this model/metadata needs saving.
    void setModelSaved( bool);
    void setMetaSaved( bool);
    bool isMetaSaved() const { return _savedMeta;}
    bool isModelSaved() const { return _savedModel;}
    bool isSaved() const { return isMetaSaved() && isModelSaved();}

    // The views associated with this model.
    const FVS& fvs() const { return _fvs;}

    // Returns true if any of the metadata are present.
    bool hasMetaData() const;

    // Find and return point on surface closest to the given point
    // (which may not be on the surface). This is a transformed point.
    Vec3f findClosestSurfacePoint( const Vec3f&) const;

    // Use the KD-tree to find the vertex index closest to the given (transformed) position.
    int findVertex( const Vec3f&) const;

    // Translate the given point to the surface of this model. First finds the
    // closest point on the surface using the internal kd-tree.
    float toSurface( Vec3f&) const;

    void addView( Vis::FaceView*);
    void eraseView( Vis::FaceView*);

    static QString LENGTH_UNITS;
    static int MAX_MANIFOLDS;   // For new FaceModel's the per model max num 2D triangulated manifolds.

private:
    bool _savedMeta;
    bool _savedModel;

    QString _source;    // Image source
    QString _studyId;   // Study ID info
    QDate _dob;         // Subject date of birth
    int8_t _sex;        // Subject sex
    int _methnicity;    // Subject's maternal ethnicity
    int _pethnicity;    // Subject's paternal ethnicity
    QDate _cdate;       // Date of image capture

    r3d::Mesh::Ptr _mesh;
    r3d::Manifolds::Ptr _manifolds;
    r3d::KDTree::Ptr _kdtree;

    std::vector<r3d::Bounds::Ptr> _bnds;

    r3d::Mesh::Ptr _mask;
    r3d::KDTree::Ptr _mkdtree;
    size_t _maskHash;

    QMap<int, FaceAssessment::Ptr> _ass;    // Assessments keyed by id
    FaceAssessment::Ptr _cass;              // Current assessment

    mutable QReadWriteLock _mutex;
    FVS _fvs;  // Associated FaceViews

    friend class Vis::FaceView;
    friend class Action::FaceModelState;

    bool _moveToSurface();
    void _syncBoundsToAlignment();
    FaceModel( const FaceModel&) = delete;
    void operator=( const FaceModel&) = delete;
};  // end class

}   // end namespace

Q_DECLARE_METATYPE( r3d::Mesh::Ptr)

#endif
