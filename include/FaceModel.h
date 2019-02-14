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
#include <Orientation.h>
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

    // Set/get orientation of the face.
    void setOrientation( const RFeatures::Orientation &o) { if (_orientation != o) setSaved(false); _orientation = o;}
    const RFeatures::Orientation& orientation() const { return _orientation;}

    // Set/get "centre" of the face.
    void setCentre( const cv::Vec3f& c) { if ( _centre != c) setSaved(false); _centre = c; _centreSet = true;}
    const cv::Vec3f& centre() const { return _centre;}
    bool centreSet() const { return _centreSet;}    // True iff setCentre has been called.

    Landmark::LandmarkSet::Ptr landmarks() const { return _landmarks;}    // CALL setSaved(false) AFTER UPDATING!
    PathSet::Ptr paths() const { return _paths;}    // CALL setSaved(false) AFTER UPDATING!

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

    // Set/get age of individual.
    void setAge( double a) { _age = a;}
    double age() const { return _age;}

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

    static QString LENGTH_UNITS;

private:
    bool _saved;
    QString _notes;   // Long form description
    QString _source;        // Data source info
    QString _studyId;       // Study ID info
    double _age;
    int8_t _sex;
    QString _ethnicity;
    QDate _cdate;               // Date of image capture.
    bool _centreSet;            // If face centre has been set.
    cv::Vec3f _centre;          // Face "centre"
    RFeatures::Orientation _orientation;
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
