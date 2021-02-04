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

#ifndef FACE_TOOLS_METRIC_METRIC_MANAGER_H
#define FACE_TOOLS_METRIC_METRIC_MANAGER_H

#include "Metric.h"

namespace FaceTools { namespace Metric {

using MC = Metric;
using MCSet = std::unordered_set<MC::Ptr>;

class FaceTools_EXPORT MetricManager
{
public:
    // Parse the files in the given directory to create Metric instances.
    // Returns the number of Metrics created. If Metrics with the
    // same id already exist, they are overridden. On error, returns < 0.
    static int load( const QString&);

    // Return the number of metric's available.
    static size_t count() { return _metrics.size();}

    // Returns alphanumerically sorted list of unique metric names.
    static const QStringList& names() { return _names;}

    static const MCSet& metrics() { return _mset;}
    static const IntSet& ids() { return _ids;}
    static const IntSet& bilateralIds() { return _bids;}

    // Returns all metrics only for the given landmark (may return empty set).
    static const IntSet& metricsForLandmark( int lmid);

    // Returns only those metrics with visualisations defined.
    static const MCSet& visMetrics() { return _vmset;}

    // Return the metric with given name or null if it doesn't exist.
    static MC::Ptr metricForName( const QString&);

    // Return the metric with given id or null if it doesn't exist.
    static MC::Ptr metric( int);

    // Return the const metric with given id or null if it doesn't exist.
    static const MC *cmetric( int);

    // Purge all metrics associated of data associated with the given model.
    static void purge( const FM*);

private:
    static IntSet _ids;
    static IntSet _bids;
    static std::unordered_map<int, MC::Ptr> _metrics;
    static std::unordered_map<int, IntSet> _lmMetrics;  // Metrics keyed by landmark
    static std::unordered_map<QString, int> _nMetrics;  // Metric IDs keyed by name
    static MCSet _mset;
    static MCSet _vmset;
    static QStringList _names;
};  // end class

}}  // end namespaces

#endif
