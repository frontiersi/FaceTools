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

#ifndef FACE_TOOLS_FACE_ASSESSMENT_H
#define FACE_TOOLS_FACE_ASSESSMENT_H

#include "LandmarkSet.h"
#include "MetricSet.h"
#include "PathSet.h"

namespace FaceTools {

class FaceTools_EXPORT FaceAssessment
{
public:
    using Ptr = std::shared_ptr<FaceAssessment>;
    using CPtr = std::shared_ptr<const FaceAssessment>;

    static Ptr create( int id, const QString& assessorName="");

    // Makes a clone of this assessment with the same details EXCEPT the
    // assessor's name and id which are given, and the notes which are cleared.
    // Landmarks, paths and metrics are deep copied over.
    Ptr createClone( int id, const QString& newAssessor);

    Ptr deepCopy() const;

    // Returns the constructor defined identifier.
    int id() const { return _id;}

    bool setAssessor( const QString&);
    const QString& assessor() const { return _assessor;}

    bool setNotes( const QString&);
    const QString& notes() const { return _notes;}
    bool hasNotes() const { return !_notes.isEmpty();}

    bool setLandmarks( Landmark::LandmarkSet::Ptr);
    const Landmark::LandmarkSet& landmarks() const { return *_landmarks;}
    bool hasLandmarks() const { return !_landmarks->empty();}

    // Set the position of the landmark with given ID.
    void setLandmarkPosition( int, FaceLateral, const cv::Vec3f&);
    bool swapLandmarkLaterals();

    // Add the given transform to landmarks and paths returning true landmarks/paths not empty.
    bool addTransform( const cv::Matx44d&);

    // Cause each landmark to update its position to be at the closest point on the surface.
    // This should always be the case normally unless manually placing a set of landmarks.
    bool moveLandmarksToSurface( const FM*);

    const PathSet& paths() const { return *_paths;}
    bool setPaths( PathSet::Ptr);
    int addPath( const cv::Vec3f&);
    bool removePath( int pid);
    bool renamePath( int pid, const QString&);
    // Set the position of the handle of path with given ID returning true on success.
    bool setPathPosition( const FM*, int pid, int handle, const cv::Vec3f& pos);
    void recalculatePaths( const FM*);
    bool hasPaths() const { return !_paths->empty();}

    Metric::MetricSet& metrics() { return _metrics;}
    Metric::MetricSet& metricsL() { return _metricsL;}
    Metric::MetricSet& metricsR() { return _metricsR;}

    // Const versions (require different name for use by Lua).
    const Metric::MetricSet& cmetrics() const { return _metrics;}
    const Metric::MetricSet& cmetricsL() const { return _metricsL;}
    const Metric::MetricSet& cmetricsR() const { return _metricsR;}

    // Is the metric with given ID recorded in this FaceAssessment?
    bool hasMetric( int mid) const;
    void clearMetrics();

    bool hasContent() const;

    ~FaceAssessment();  // Public for Lua binding

private:
    int _id;
    QString _assessor;
    QString _notes;
    Landmark::LandmarkSet::Ptr _landmarks;
    PathSet::Ptr _paths;
    Metric::MetricSet _metrics;
    Metric::MetricSet _metricsL;
    Metric::MetricSet _metricsR;

    FaceAssessment( int, const QString&);
    FaceAssessment( const FaceAssessment&);
    FaceAssessment& operator=( const FaceAssessment&);
};  // end class

}   // end namespace

#endif
