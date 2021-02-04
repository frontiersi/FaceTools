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

#ifndef FACE_TOOLS_FACE_ASSESSMENT_H
#define FACE_TOOLS_FACE_ASSESSMENT_H

#include "LndMrk/LandmarkSet.h"
#include "Metric/MetricSet.h"
#include "PathSet.h"

namespace FaceTools {

class FaceTools_EXPORT FaceAssessment
{
public:
    using Ptr = std::shared_ptr<FaceAssessment>;
    using CPtr = std::shared_ptr<const FaceAssessment>;

    static Ptr create( int id);

    Ptr deepCopy() const;

    void setId( int id) { _id = id;}
    int id() const { return _id;}

    bool setAssessor( const QString&);
    const QString& assessor() const { return _assessor;}

    bool setNotes( const QString&);
    const QString& notes() const { return _notes;}
    bool hasNotes() const { return !_notes.isEmpty();}

    bool setLandmarks( const Landmark::LandmarkSet&);
    const Landmark::LandmarkSet& landmarks() const { return _landmarks;}
    Landmark::LandmarkSet& landmarks() { return _landmarks;}
    bool hasLandmarks() const { return !_landmarks.empty();}

    void transform( const Mat4f&);

    // Resettle paths and landmarks so that they are incident with the given model's surface.
    void moveToSurface( const FM*);

    const PathSet& paths() const { return _paths;}
    PathSet& paths() { return _paths;}
    bool setPaths( const PathSet&);
    bool hasPaths() const { return !_paths.empty();}

    Metric::MetricSet& metrics( FaceSide);
    const Metric::MetricSet& cmetrics( FaceSide) const; // Const versions (different name for use by Lua).

    // Is the metric with given ID recorded in this FaceAssessment?
    bool hasMetric( int mid) const;
    void clearMetrics();

    bool hasContent() const;

    ~FaceAssessment(){};  // Public for Lua binding

private:
    int _id;
    QString _assessor;
    QString _notes;
    Landmark::LandmarkSet _landmarks;
    PathSet _paths;
    Metric::MetricSet _metrics;
    Metric::MetricSet _metricsL;
    Metric::MetricSet _metricsR;

    explicit FaceAssessment( int);
    FaceAssessment( const FaceAssessment&) = default;
    FaceAssessment& operator=( const FaceAssessment&) = default;
};  // end class

}   // end namespace

Q_DECLARE_METATYPE( FaceTools::FaceAssessment::Ptr)

#endif
