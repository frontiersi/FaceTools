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

#include <MetricSet.h>
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MetricValue;

MetricSet::Ptr MetricSet::create() { return Ptr( new MetricSet, [](MetricSet* d){ delete d;});}

void MetricSet::set( const MetricValue& m)
{
    int id = m.id();
    _metrics[id] = m;
    _ids.insert(id);
}   // end set


size_t MetricSet::add( const MetricSet& ms)
{
    for ( int id : ms.ids())
        set( ms.metric(id));
    return ms.size();
}   // end add


const MetricValue& MetricSet::metric( int id) const { return _metrics.at(id);}


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
    _metrics.clear();
    _ids.clear();
}   // end reset


void MetricSet::write( PTree& node, double age) const
{
    for ( int id : _ids)
        metric(id).write(node, age);
}   // end write
