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
#include <cassert>
#include <Round.h>  // rlib
using FaceTools::Metric::MetricValue;
using MCM = FaceTools::Metric::MetricCalculatorManager;

MetricValue::MetricValue( int id, const std::vector<double>& v, const std::vector<double>& z, const std::vector<bool>& zok)
    : _id(id)
{
    if ( !set(v, z, zok))
    {
        std::cerr << "[ERROR] FaceTools::Metric::MetricValue::ctor: Invalid metric values set!" << std::endl;
        assert(false);
    }   // end if
}   // end ctor


bool MetricValue::set( const std::vector<double>& v, const std::vector<double>& z, const std::vector<bool>& zok)
{
    MC::Ptr metric = MCM::metric(_id);
    assert(metric);
    if ( !metric)
        return false;

    const size_t dims = metric->dims(); // Required dimensions

    assert( v.size() == z.size());
    assert( z.size() == zok.size());
    assert( zok.size() == dims);
    if ( v.size() != z.size() || z.size() != zok.size() || zok.size() != dims)
        return false;

    _vals = v;
    _zscs = z;
    _zoks = zok;
    return true;
}   // end set


PTree& FaceTools::Metric::operator<<( PTree& pnode, const MetricValue& m)
{
    MC::Ptr mc = MCM::metric( m.metricId());

    PTree& mnode = pnode.add("MetricValue","");
    mnode.put( "<xmlattr>.name", mc->name().toStdString());
    mnode.put( "<xmlattr>.id", mc->id());
    mnode.put( "category", mc->category().toStdString());
    const int ndps = static_cast<int>(mc->numDecimals());
    const size_t dims = mc->dims();
    mnode.put( "ndims", dims);

    PTree& vnode = mnode.put( "values", "");
    for ( size_t i = 0; i < dims; ++i)
    {
        std::ostringstream ossd;
        ossd << "dim_" << i;
        PTree& dnode = vnode.put( ossd.str(), "");
        dnode.put( "measure", rlib::dps( m.values()[i], ndps));
        dnode.put( "zscore", rlib::dps( m.zscores()[i], ndps));
        dnode.put( "zokay", m.zokays()[i]);
    }   // end for
    return pnode;
}   // end write


const PTree& FaceTools::Metric::operator>>( const PTree& mnode, MetricValue& m)
{
    m.setMetricId( mnode.get<int>( "<xmlattr>.id"));
    const size_t dims = mnode.get<size_t>( "ndims");

    MC::Ptr mc = MCM::metric( m.metricId());
    assert( mc);
    if ( !mc)
    {
        std::cerr << "[ERROR] FaceTools::Metric::operator>>: MetricValue; metric ID not found!" << std::endl;
        return mnode;
    }   // end if

    assert( dims == mc->dims());
    if ( dims != mc->dims())
    {
        std::cerr << "[ERROR] FaceTools::Metric::operator>>: MetricValue; metric dimension mismatch!" << std::endl;
        return mnode;
    }   // end if

    std::vector<double> vs(dims);
    std::vector<double> zs(dims);
    std::vector<bool> zok(dims);
    const PTree& vnode = mnode.get_child("values");
    for ( size_t i = 0; i < dims; ++i)
    {
        std::ostringstream ossd;
        ossd << "dim_" << i;
        const PTree& dnode = vnode.get_child(ossd.str());
        vs[i]  = dnode.get<double>("measure");
        zs[i]  = dnode.get<double>("zscore");
        zok[i] = dnode.get<bool>("zokay");
    }   // end for
    m.set( vs, zs, zok);

    return mnode;
}   // end operator>>
