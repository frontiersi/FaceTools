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

#ifndef FACE_TOOLS_FACE_MODEL_H
#define FACE_TOOLS_FACE_MODEL_H

#include "LandmarkSet.h"
#include "MetricSet.h"
#include "PathSet.h"
#include "FaceViewSet.h"
#include <ObjModelTools.h>   // RFeatures
#include <QReadWriteLock>
#include <QDate>

namespace FaceTools {

class FaceTools_EXPORT FaceModel
{
public:
    explicit FaceModel( RFeatures::ObjModel::Ptr);
    FaceModel();

    void lockForWrite();        // Lock before making write changes to this FaceModel.
    void lockForRead() const;   // Lock before reading this FaceModel's state.
    void unlock() const;        // Call after done with read or write locks.

    /**
     * Update with new model or refresh with the existing if model parameter null.
     * This function sets a new model and calls fixTransform on it. It also reseats
     * landmarks on the given model and recalculates the paths.
     * If updateConnectivity is true, model manifolds are reparsed which
     * is an expensive operation. Pass false if not needed.
     * View actors should be rebuild after calling this function.
     */
    void update( RFeatures::ObjModel::Ptr=nullptr, bool updateConnectivity=true);

    /**
     * For making rigid affine changes to the data that can be expressed using a matrix.
     * Transform the model, search tree, landmarks and paths using the given matrix
     * as well as all FaceViews associated with this model. Note that only the landmarks
     * and paths are actually updated directly; the model, kd-tree, and bounds are set
     * with matrices to transform their vertices.
     */
    void addTransformMatrix( const cv::Matx44d&);

    /**
     * Fix this model's orientation either to the current model transform (if no landmarks)
     * or to the orientation of the landmarks (if available). As well as fixing the model
     * transform, this also rebuilds the KD-tree and recalculates bounding boxes.
     * View actors should be rebuilt after calling this function.
     */
    void fixOrientation();

    /**
     * Updating the shared model reference inplace requires calling update afterwards
     * to ensure all changes to the model and its associated data propogate through.
     * If simply wanting to perform an affine transform, call the transform function.
     */
    RFeatures::ObjModel::Ptr wmodel() { return _model;}

    const RFeatures::ObjModel& model() const { return *_model;}
    const RFeatures::ObjModelKDTree& kdtree() const { return *_kdtree;}
    const RFeatures::ObjModelManifolds& manifolds() const { return *_manifolds;}

    /**
     * Returns full model bounds at entry zero, and the corresponding manifold bounds at higher indices.
     * Without landmarks, the bounds at entry zero encompass the model. With landmarks, the bounds at
     * entry zero are defined around the head and the orientation and placement of the bounds is governed
     * by the landmark placement.
     */
    const std::vector<RFeatures::ObjModelBounds::Ptr>& bounds() const { return _bnds;}

    const Landmark::LandmarkSet& landmarks() const { return *_landmarks;}
    void setLandmarks( Landmark::LandmarkSet::Ptr);
    bool hasLandmarks() const { return !_landmarks->empty();}
    // Set the position of the landmark with given ID returning true on success.
    bool setLandmarkPosition( int lid, const cv::Vec3f& pos, FaceLateral);
    void swapLandmarkLaterals();

    // Cause each landmark to update its position to be at the closest point on the surface.
    // This should always be the case normally unless manually placing a set of landmarks.
    void moveLandmarksToSurface();

    const PathSet& paths() const { return *_paths;}
    void setPaths( PathSet::Ptr);
    int addPath( const cv::Vec3f&);
    bool removePath( int pid);
    bool hasPaths() const { return !_paths->empty();}
    // Rename the path with given ID returning true if the path exists and was renamed.
    bool renamePath( int pid, const QString&);
    // Set the position of the handle of path with given ID returning true on success.
    bool setPathPosition( int pid, const cv::Vec3f& pos, int handle=0);

    // Returns the ID of the manifold holding the face or -1 if a medial landmark doesn't exist.
    int faceManifoldId() const;

    Metric::MetricSet& metrics() { return _metrics;}
    const Metric::MetricSet& cmetrics() const { return _metrics;}
    Metric::MetricSet& metricsL() { return _metricsL;}
    const Metric::MetricSet& cmetricsL() const { return _metricsL;}
    Metric::MetricSet& metricsR() { return _metricsR;}
    const Metric::MetricSet& cmetricsR() const { return _metricsR;}

    // Is the metric with given ID recorded in this FaceModel?
    bool hasMetric( int mid) const;

    void clearMetrics();

    // Set/get image notes.
    void setNotes( const QString&);
    const QString& notes() const { return _notes;}

    // Set/get source of data.
    void setSource( const QString&);
    const QString& source() const { return _source;}

    // Set/get study ID
    void setStudyId( const QString&);
    const QString& studyId() const { return _studyId;}

    // Get age of individual at date of image capture.
    double age() const { return double( dateOfBirth().daysTo( captureDate())) / 365.25;}

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

    // Convenience function to update renderers on all associated FaceViews.
    void updateRenderers() const;

    // Find and return the point on the surface closest to the given point (which may not be on the surface).
    cv::Vec3f findClosestSurfacePoint( const cv::Vec3f&) const;

    // Use the KD-tree to find the vertex index closest to the given position.
    int findVertex( const cv::Vec3f&) const;

    // Translate the given point to the surface of this model. First finds the
    // closest point on the surface using the internal kd-tree.
    double translateToSurface( cv::Vec3f&) const;

    // Return the centre and orientation of this model defined by the landmarks if set,
    // or from the underlying model's transform matrix if no landmarks are set.
    // Use bounds() to get the centre of the model's bounding box.
    cv::Vec3f icentre() const;
    RFeatures::Orientation orientation() const;

    void addView( Vis::FaceView*);
    void eraseView( Vis::FaceView*);

    static QString LENGTH_UNITS;
    static int MAX_MANIFOLDS;   // For new FaceModel's the per model maximum number of 2D triangulated manifolds.

private:
    bool _savedMeta;
    bool _savedModel;
    QString _notes;     // Image notes
    QString _source;    // Image source
    QString _studyId;   // Study ID info
    QDate _dob;         // Subject date of birth
    int8_t _sex;        // Subject sex
    int _methnicity;    // Subject's maternal ethnicity
    int _pethnicity;    // Subject's paternal ethnicity
    QDate _cdate;       // Date of image capture
    Landmark::LandmarkSet::Ptr _landmarks;
    PathSet::Ptr _paths;
    Metric::MetricSet _metrics;
    Metric::MetricSet _metricsL;
    Metric::MetricSet _metricsR;
    RFeatures::ObjModel::Ptr _model;
    RFeatures::ObjModelManifolds::Ptr _manifolds;
    RFeatures::ObjModelKDTree::Ptr _kdtree;
    std::vector<RFeatures::ObjModelBounds::Ptr> _bnds;

    mutable QReadWriteLock _mutex;
    FVS _fvs;  // Associated FaceViews
    friend class Vis::FaceView;
    friend class Action::UndoState;

    void _syncOrientationBounds();
    void _makeOrientationBounds();
    FaceModel( const FaceModel&) = delete;
    void operator=( const FaceModel&) = delete;
};  // end class

}   // end namespace

#endif
