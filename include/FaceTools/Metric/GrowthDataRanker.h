/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
    void combineEthnicities();

    bool empty() const { return _gdata.empty();}
    const GrowthDataSources& all() const { return _all;}

    // Return the available sexs
    std::unordered_set<int8_t> sexes() const;

    // Return the available ethnicities
    IntSet ethnicities() const;

    // Return the source references sorted alphanumerically
    static QStringList sources( const GrowthDataSources&);

    // Return the matched GrowthData sources or an empty set.
    GrowthDataSources lookup( int8_t sex, int ethn) const;

    // Returns true iff data exist for the given sex, ethnicity.
    bool hasData( int8_t sex, int ethn) const;

    // Return the match GrowthData or null.
    const GrowthData* lookup( int8_t sex, int ethn, const QString& src) const;

    const GrowthData* current() const { return _cgd;}
    void setCurrent( const GrowthData*);    // Set the current explicitly

    // Returns just those set as compatible from the last call to set.
    const GrowthDataSources& compatible() const { return _compat;}
    void setCompatible( const FM*); // Set the most compatible set for the given model.

    // Return the best scored match for the given model. Won't be null unless this ranker is empty.
    const GrowthData* findBestMatch( const FM*) const;

    const GrowthData* findBestMatch( int8_t sex, int ethn) const;

    const GrowthData* stats( int growthDataId) const;

private:
    int _gids;
    const GrowthData *_cgd;
    std::unordered_map<int8_t, std::unordered_map<int, GrowthDataSources> > _gdata; // Keyed by sex-->ethnicity
    std::unordered_map<int8_t, GrowthDataSources> _sdata;   // Keyed by sex
    GrowthDataSources _compat;
    GrowthDataSources _all;
    std::unordered_set<GrowthData::Ptr> _allptrs;
    std::unordered_map<int, const GrowthData*> _stats;

    const GrowthData* _findBestMatch( int8_t, int, int, float) const;
    GrowthDataSources _findMatching( int8_t, int) const;
    void _findMatching( int8_t, int, GrowthDataSources&) const;
    GrowthDataRanker( const GrowthDataRanker&) = delete;
    void operator=( const GrowthDataRanker&) = delete;
};  // end class

}}   // end namespaces

#endif
