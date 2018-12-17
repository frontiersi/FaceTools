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

#include <MetricValue.h>
#include <MetricCalculatorManager.h>
#include <Round.h>  // rlib
#include <cassert>
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::DimensionStat;
using MCM = FaceTools::Metric::MetricCalculatorManager;


namespace {
void checkId( int id)
{
    FaceTools::Metric::MC::Ptr metric = MCM::metric(id);
    if ( !metric)
    {
        std::cerr << "[ERROR] FaceTools::Metric::MetricValue::ctor: Invalid metric values set!" << std::endl;
        assert(metric);
    }   // end if
}   // end checkId
}   // end namespace


MetricValue::MetricValue( int id)
    : _id(id), _sex(0), _eth(""), _src("")
{
    checkId(id);
}   // end ctor


MetricValue::MetricValue( int id, const std::vector<DimensionStat>& ds)
    : _id(id), _sex(0), _eth(""), _src(""), _dstats(ds)
{
    checkId(id);
    MC::Ptr mc = MCM::metric( id);
    const size_t rdims = mc->dims();    // Required number of dimensions
    if ( ndims() > rdims)
    {
        _dstats.resize(rdims);  // Truncate!
        std::cerr << "[ERROR] FaceTools::Metric::MetricValue::ctor: Too many statistics added for the allowed number of dimensions!" << std::endl;
        assert(false);
    }   // end if
}   // end ctor


MetricValue::MetricValue( const MetricValue& mv)
{
    *this = mv;
}   // end ctor


MetricValue& MetricValue::operator=( const MetricValue& mv)
{
    _id = mv._id;
    _sex = mv._sex;
    _eth = mv._eth;
    _src = mv._src;
    _dstats = mv._dstats;
    return *this;
}   // end operator=


bool MetricValue::addStat( const DimensionStat& ds)
{
    MC::Ptr mc = MCM::metric( id());
    const size_t rdims = mc->dims();    // Required number of dimensions
    if ( ndims() >= rdims)
        return false;
    _dstats.push_back(ds);
    return true;
}   // end addStat


PTree& FaceTools::Metric::operator<<( PTree& pnode, const MetricValue& m)
{
    MC::Ptr mc = MCM::metric( m.id());

    PTree& mnode = pnode.add("MetricValue","");

    mnode.put( "<xmlattr>.id", mc->id());
    mnode.put( "<xmlattr>.name", mc->name().toStdString());
    mnode.put( "category", mc->category().toStdString());
    const int ndps = static_cast<int>(mc->numDecimals());
    const size_t dims = mc->dims();
    mnode.put( "ndims", dims);
    mnode.put( "sex", FaceTools::toLongSexString(m.sex()).toStdString());
    mnode.put( "ethnicity", m.ethnicity());
    mnode.put( "source", m.source());

    // Output the statistics for the metric
    PTree& dnode = mnode.put( "stats", "");
    for ( size_t i = 0; i < dims; ++i)
    {
        PTree& node = dnode.add( "dimension", "");
        node.put( "axis", i);
        node.put( "value",  rlib::dps( m.value(i),  ndps));
        node.put( "zscore", rlib::dps( m.zscore(i), ndps));
        node.put( "mean",   rlib::dps( m.mean(i),   ndps));
        node.put( "stdv",   rlib::dps( m.stdv(i),   ndps));
    }   // end for

    return pnode;
}   // end write
