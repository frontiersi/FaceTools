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

#ifndef FACE_TOOLS_METRIC_METRIC_SET_H
#define FACE_TOOLS_METRIC_METRIC_SET_H

#include "MetricValue.h"
#include <unordered_map>
#include <unordered_set>
using IntSet = std::unordered_set<int>;

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricSet
{
public:
    using Ptr = std::shared_ptr<MetricSet>;
    static Ptr create();

    MetricSet(){}
    MetricSet( const MetricSet&) = default;
    MetricSet& operator=( const MetricSet&) = default;
    ~MetricSet(){}

    // Copy in metric (or overwrite if has same metric id).
    void set( const MetricValue&);

    // Copy in given set of metrics to this set returning num added.
    size_t add( const MetricSet&);

    // Returns metric or null if no metric with id exists.
    const MetricValue& metric( int id) const;

    // Returns true if metric existed and was removed.
    bool erase( int id);

    // Clears all metrics from this set.
    void reset();

    // Get metric ids.
    const IntSet& ids() const { return _ids;}

    // Set membership tests
    bool empty() const { return count() == 0;}
    size_t count() const { return _metrics.size();}
    size_t size() const { return _metrics.size();}
    bool has( int id) const { return _ids.count(id) > 0;}

    void write( PTree& node, double age) const;

private:
    std::unordered_map<int, MetricValue> _metrics;
    IntSet _ids;
};  // end class

}}   // end namespace

#endif
