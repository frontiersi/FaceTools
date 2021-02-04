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

#ifndef FACE_TOOLS_METRIC_METRIC_TYPE_H
#define FACE_TOOLS_METRIC_METRIC_TYPE_H

#include <FaceTools/Vis/MetricVisualiser.h>
#include <FaceTools/LndMrk/LandmarkSet.h>
#include <FaceTools/FaceModel.h>

namespace FaceTools { namespace Metric {


struct FaceTools_EXPORT MetricParams
{
    int id;             // Metric identifier.
    bool bilateral;     // Measured on both laterals of the face?
    QString normal;     // Plane normal for projection ("x", "y", "z").
    QString remarks;    // Remarks about the metric's definition.

    size_t dimensions() const { return points.size();}

    // Landmarks defining the bounds. Outer vector is dimensions,
    // inner vector is is # points, LmkList are the landmark(s) specifying the point.
    std::vector< std::vector<Landmark::LmkList> > points;
    std::vector< std::vector<Landmark::LmkList> > spoints;  // No need to explictly set
};  // end MetricParams


class FaceTools_EXPORT MetricType
{
public:
    using Ptr = std::shared_ptr<MetricType>;

    virtual ~MetricType(){}
    void setParams( const MetricParams&);

    virtual QString category() const = 0;
    virtual QString units() const { return "";} // Depends on category
    virtual QString typeRemarks() const { return "";}   // Remarks for the metric type
    virtual size_t dimensions() const { return _prms.dimensions();}
   
    // Return the visualisation for this metric (if any).
    virtual Vis::MetricVisualiser *visualiser() { return nullptr;}

    int id() const { return _prms.id;}
    bool bilateral() const { return _prms.bilateral;}
    const QString &remarks() const { return _prms.remarks;}
    const QString &normal() const { return _prms.normal;}   // "x", "y", "z"

    // Return whether or not this metric is always calculated in-plane.
    // Measurements are assumed to be fixed in plane by default.
    virtual bool fixedInPlane() const { return true;}

    // Return plane projection normal for given model's current assessment.
    Vec3f normal( const FM*) const;

    // Return the set of landmarks used (their IDs).
    const IntSet &landmarkIds() const { return _lmids;}

    // Measure against the given model for its current assessment. Output values are placed
    // into out parameter results with as many entries as there are measurement dimensions.
    // The inPlane option is only considered if this metric is not fixedInPlane().
    void measure( std::vector<float> &results, const FM*, bool swapSide, bool inPlane);

    // Get this list of points for dimension i. Use swapped=true if this is a bilateral metric
    // and want the points defined for the subject's right face lateral.
    const std::vector<Landmark::LmkList>& points( size_t i, bool swapped=false) const;

    // Returns true iff measurement recorded for the given model.
    virtual bool hasMeasurement( const FM*) const = 0;

    // Purge this metric of any data cached for the given model.
    virtual void purge( const FM*) {}

protected:
    // From the given model and points, projection plane vector, and flag saying whether or not to
    // project (ignored if fixedInPlane()), calculate and return the measurement value for dimension
    // dim. Child classes should update cache of measurements for the given model if need be. If
    // swapped is true, then this is a bilateral metric and the measurement needed is for the other
    // side of the face.
    virtual float update( size_t dim, const FM*, const std::vector<Vec3f>&, Vec3f, Vec3f, bool, bool) = 0;

private:
    MetricParams _prms;
    IntSet _lmids;
    const std::vector<Landmark::LmkList>& _origPoints( size_t i) const { return _prms.points.at(i);}
    const std::vector<Landmark::LmkList>& _swapPoints( size_t i) const { return _prms.spoints.at(i);}
};  // end class

}}   // end namespaces

#endif
