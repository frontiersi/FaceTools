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

#ifndef FACE_TOOLS_METRIC_STATS_MANAGER_H
#define FACE_TOOLS_METRIC_STATS_MANAGER_H

#include "GrowthData.h"
#include <FaceTools/FaceModel.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT StatsManager
{
public:
    using RPtr = std::shared_ptr<const GrowthData>;

    static int load( const QString&);

    // Return the metric's stats for the given model or the default metric stats.
    // Returned pointer holds a read lock on this class until destroyed.
    static RPtr stats( int mid, const FM*);

    // Update the stats to use for the given model - automatically choosing the best for it.
    // Can call from a separate thread since might take a few moments.
    static void updateStatsForModel( const FM&);

    // Purge data associated with the given model.
    static void purge( const FM&);

    // Set default stats for the given metric.
    static void setDefaultMetricStats( int mid, int8_t sex, int ethn, const QString &src);
    static void setDefaultMetricStats( int mid, const GrowthData*);

    // Returns the default metric stats for the given metric or null if not set.
    static const GrowthData* defaultMetricStats( int mid);

    // Returns true if default stats being used for the given metric.
    static bool usingDefaultMetricStats( int mid);

    // Set whether to use the metric's default or model specific statistics.
    static void setUseDefaultMetricStats( int mid, bool);

private:
    // Models to metric IDs and their associated growth data mappings.
    static std::unordered_map<const FM*, std::unordered_map<int, const GrowthData*> > _modelGDs;
    static std::unordered_map<int, const GrowthData*> _metricGDs;
    static IntSet _metricDefaults;
    static QReadWriteLock _lock;
};  // end class

}}   // end namespaces

#endif
