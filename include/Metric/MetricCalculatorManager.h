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

#ifndef FACE_TOOLS_METRIC_METRIC_CALCULATOR_MANAGER_H
#define FACE_TOOLS_METRIC_METRIC_CALCULATOR_MANAGER_H

#include "MetricCalculator.h"

namespace FaceTools { namespace Metric {

using MCSet = std::unordered_set<MC::Ptr>;

class FaceTools_EXPORT MetricCalculatorManager
{
public:
    // Parse the files in the given directory to create MetricCalculator instances.
    // Returns the number of MetricCalculators created. If MetricCalculators with the
    // same id already exist, they are overridden. On error, returns < 0.
    static int load( const QString&);

    // Return the number of metric calculator's available.
    static size_t count() { return _metrics.size();}

    // Returns alphanumerically sorted list of unique metric names.
    static const QStringList& names() { return _names;}

    static const IntSet& ids() { return _ids;}

    // Returns all metrics.
    static const MCSet& metrics() { return _mset;}

    // Returns only those metrics with visualisations defined.
    static const MCSet& vmetrics() { return _vmset;}

    // Return the metric calculator with given id or null if it doesn't exist.
    static MC::Ptr metric( int);

    // Return the currently selected metric (null if none yet loaded).
    static MC::Ptr currentMetric();

    // Return the metric selected prior to the currently selected one (null if none prior).
    static MC::Ptr previousMetric();

    // Set the current metric - firing it's activated signal if different from the existing activated.
    // Returns true only if a new metric was activated (false if mid same as currently selected).
    static bool setCurrentMetric( int mid);

private:
    static IntSet _ids;
    static std::unordered_map<int, MC::Ptr> _metrics;
    static MCSet _mset;
    static MCSet _vmset;
    static QStringList _names;
    static int _cmid;
    static int _pmid;
};  // end class

}}  // end namespaces

#endif
