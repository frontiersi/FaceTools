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

#include <MetricSet.h>
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MetricValue;
#include <algorithm>
#include <iostream>

MetricSet::Ptr MetricSet::create() { return Ptr( new MetricSet, [](MetricSet* d){ delete d;});}

void MetricSet::set( const MetricValue& m)
{
    int id = m.metricId();
    _metrics[id] = m;
    _ids.insert(id);
}   // end set


size_t MetricSet::add( const MetricSet& ms)
{
    for ( int id : ms.ids())
        set( *ms.get(id));
    return ms.size();
}   // end add


const MetricValue* MetricSet::get( int id) const { return has(id) ? &_metrics.at(id) : nullptr;}


bool MetricSet::erase( int id)
{
    if (!has(id))
        return false;
    _metrics.erase(id);
    _ids.erase(id);
    return true;
}   // end erase


void MetricSet::reset()
{
    IntSet ids = _ids;
    for ( int id : ids)
        erase(id);
}   // end reset


PTree& FaceTools::Metric::operator<<( PTree& mnodes, const MetricSet& ms)
{
    const IntSet& ids = ms.ids();
    std::for_each( std::begin(ids), std::end(ids), [&](int id){ mnodes << *ms.get(id);});
    return mnodes;
}   // end operator<<


const PTree& FaceTools::Metric::operator>>( const PTree& mnodes, MetricSet& ms)
{
    for ( const PTree::value_type& mnode : mnodes)
    {
        if ( mnode.first == "Metric")
        {
            MetricValue m;
            mnode.second >> m;
            ms.set( m);
        }   // end if
    }   // end foreach
    return mnodes;
}   // end operator>>
