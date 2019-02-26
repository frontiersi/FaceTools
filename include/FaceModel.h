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
    explicit FaceModel( RFeatures::ObjModelInfo::Ptr);
    FaceModel();

    // Use these read/write locks before accessing or writing to this FaceModel.
    void lockForWrite();
    void lockForRead() const;
    void unlock() const;    // Call after done with read or write locks.

    // Update with new data. Returns false iff a null object is passed in or the wrapped ObjModel is not a
    // valid 2D manifold. If parameter NULL, update with existing data (which is presumed to have changed).
    bool update( RFeatures::ObjModelInfo::Ptr=nullptr);

    // For making linear changes to the model that can be expressed using a matrix.
    // Transform the model, the orientation, and the landmarks using the given matrix.
    void transform( const cv::Matx44d&);

    // Use this function to access the model for making direct changes. After making
    // changes, call update to ensure that updates propagate through. If making changes
    // to the wrapped ObjModel, ensure that ObjModelInfo::reset is called before update.
    RFeatures::ObjModelInfo::Ptr info() const { return _minfo;}

    // Get the KD-tree.
    const RFeatures::ObjModelKDTree* kdtree() const { return _kdtree.get();}

    // Returns boundary values for each model component as [xmin,xmax,ymin,ymax,zmin,zmax].
    const std::vector<cv::Vec6d>& bounds() const { return _cbounds;}

    // Returns the super boundary of this model i.e. the smallest boundary
    // in 3D that contains all of this model's components.
    const cv::Vec6d& superBounds() const { return _sbounds;}

    // Calculate and return the centre of the super bounds.
    cv::Vec3f superBoundsCentre() const;

    // CALL setSaved(false) AFTER UPDATING!
    Landmark::LandmarkSet::Ptr landmarks() const { return _landmarks;}

    bool hasLandmarks() const { return !_landmarks->empty();}

    // CALL setSaved(false) AFTER UPDATING!
    PathSet::Ptr paths() const { return _paths;}

    bool hasPaths() const { return !_paths->empty();}

    Metric::MetricSet& metrics() { return _metrics;}
    const Metric::MetricSet& cmetrics() const { return _metrics;}

    Metric::MetricSet& metricsL() { return _metricsL;}
    const Metric::MetricSet& cmetricsL() const { return _metricsL;}

    Metric::MetricSet& metricsR() { return _metricsR;}
    const Metric::MetricSet& cmetricsR() const { return _metricsR;}

    const IntSet& phenotypes() const { return _phenotypes;}
    void clearPhenotypes();
    void addPhenotype( int);
    void removePhenotype( int);

    // Set/get image notes.
    void setNotes( const QString& n) { _notes = n;}
    const QString& notes() const { return _notes;}

    // Set/get source of data.
    void setSource( const QString& s) { _source = s;}
    const QString& source() const { return _source;}

    // Set/get study ID
    void setStudyId( const QString& s) { _studyId = s;}
    const QString& studyId() const { return _studyId;}

    // Get age of individual.
    double age() const { return double(_dob.daysTo(QDate::currentDate())) / 365.25;}

    // Set/get DoB of individual.
    void setDateOfBirth( const QDate& d) { _dob = d;}
    const QDate& dateOfBirth() const { return _dob;}

    // Set/get sex of individual.
    void setSex( int8_t s) { _sex = s;}
    int8_t sex() const { return _sex;}

    // Set/get ethnicity of individual.
    void setEthnicity( const QString& t) { _ethnicity = t;}
    const QString& ethnicity() const { return _ethnicity;}

    // Set/get capture date of image.
    void setCaptureDate( const QDate& d) { _cdate = d;}
    const QDate& captureDate() const { return _cdate;}

    // Set/get if this model needs saving.
    bool isSaved() const { return _saved;}
    void setSaved( bool s=true) { _saved = s;}

    // Clears orientation, metrics, and bounds info.
    void clearMeta();

    // The views associated with this model.
    const FVS& fvs() const { return _fvs;}

    void pokeTransform( vtkMatrix4x4*);

    // Returns true if any of the metadata are present.
    bool hasMetaData() const;

    // Convenience function to update renderers on all associated FaceViews.
    void updateRenderers() const;

    // Find and return the point on the surface closest to the given point (which may not be on the surface).
    cv::Vec3f findClosestSurfacePoint( const cv::Vec3f&) const;

    // Does this model's super bounds intersect with the other model's super bounds?
    bool supersIntersect( const FaceModel&) const;

    // Translate the given point to the surface of this model. First finds the
    // closest point on the surface using the internal kd-tree.
    double translateToSurface( cv::Vec3f&) const;

    // Set/get initial orientation (defaults to norm <0,0,1> and up <0,1,0>)
    void setInitialOrientation( const RFeatures::Orientation&);
    const RFeatures::Orientation& initialOrientation() const { return _iorientation;}

    // Set/get initial centre (defaults to <0,0,0>)
    void setInitialCentre( const cv::Vec3f&);
    const cv::Vec3f& initialCentre() const { return _icentre;}

    // Return the centre and orientation of this model.
    // If landmarks are set, use the orientation and centre return from those,
    // otherwise use the model's initial centre and orientation.
    cv::Vec3f centre() const;
    RFeatures::Orientation orientation() const;

    static QString LENGTH_UNITS;

private:
    bool _saved;
    QString _notes;     // Image notes
    QString _source;    // Image source
    QString _studyId;   // Study ID info
    QDate _dob;         // Subject date of birth
    int8_t _sex;        // Subject sex
    QString _ethnicity; // Subject ethnicity
    QDate _cdate;       // Date of image capture
    cv::Vec3f _icentre;                     // Initial (original) centre
    RFeatures::Orientation _iorientation;   // Inital (original) orientation
    Landmark::LandmarkSet::Ptr _landmarks;
    PathSet::Ptr _paths;
    Metric::MetricSet _metrics;
    Metric::MetricSet _metricsL;
    Metric::MetricSet _metricsR;
    IntSet _phenotypes;
    RFeatures::ObjModelInfo::Ptr _minfo;
    RFeatures::ObjModelKDTree::Ptr _kdtree;
    std::vector<cv::Vec6d> _cbounds;
    cv::Vec6d _sbounds;

    mutable QReadWriteLock _mutex;
    FVS _fvs;  // Associated FaceViews
    friend class Vis::FaceView;

    void fixTransform( vtkMatrix4x4*);
    void calculateBounds();
    void updateMeta();
    FaceModel( const FaceModel&) = delete;
    void operator=( const FaceModel&) = delete;
};  // end class

}   // end namespace

#endif
