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

#ifndef FACE_TOOLS_METRIC_GROWTH_DATA_RANKER_H
#define FACE_TOOLS_METRIC_GROWTH_DATA_RANKER_H

#include "GrowthData.h"

namespace FaceTools { namespace Metric {

using GrowthDataSources = std::unordered_set<const GrowthData*>;

class FaceTools_EXPORT GrowthDataRanker
{
public:
    GrowthDataRanker();

    void add( GrowthData::Ptr);
    void combineSexes();

    const GrowthDataSources& all() const { return _all;}
    std::unordered_set<int8_t> sexes() const; // Return the available sexs
    IntSet ethnicities() const; // Return the available ethnicities

    const GrowthData* stats( int growthDataId) const;

    // Returns the compatible set for the given model.
    GrowthDataSources compatible( const FM*) const;

    // Return the compatible set for the given sex and ethnicity.
    GrowthDataSources compatible( int8_t sex, int ethn) const;

    // Return the matching GrowthData or null.
    const GrowthData* matching( int8_t sex, int ethn, const QString& src) const;

    // Return the best scored match for the given data from the given sources.
    static const GrowthData* bestMatch( const GrowthDataSources&, const FM*);
    static const GrowthData* bestMatch( const GrowthDataSources&, int8_t sex, int eth);
    static const GrowthData* bestMatch( const GrowthDataSources&, int8_t sex, int meth, int peth, float age);
    // Return the references sorted alphanumerically for the given sources.
    static QStringList sources( const GrowthDataSources&);

private:
    int _gids;
    std::unordered_map<int8_t, std::unordered_map<int, GrowthDataSources> > _gdata; // sex-->ethnicity
    std::unordered_map<int8_t, GrowthDataSources> _sdata;   // sex
    GrowthDataSources _all;
    std::unordered_set<GrowthData::Ptr> _allptrs;
    std::unordered_map<int, const GrowthData*> _stats;

    GrowthDataSources _compatible( int8_t, int) const;
    void _compatible( int8_t, int, GrowthDataSources&) const;

    GrowthDataRanker( const GrowthDataRanker&) = delete;
    void operator=( const GrowthDataRanker&) = delete;
};  // end class

}}   // end namespaces

#endif
