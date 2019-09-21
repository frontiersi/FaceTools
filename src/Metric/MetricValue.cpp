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

#include <Metric/MetricValue.h>
#include <Metric/MetricCalculatorManager.h>
#include <Ethnicities.h>
#include <Round.h>  // rlib
#include <cassert>
using FaceTools::Metric::MetricValue;
using MCM = FaceTools::Metric::MetricCalculatorManager;


MetricValue::MetricValue( int id) : _id(id) {}


double MetricValue::zscore( double age, size_t i) const
{
    double zs = 0;
    GrowthData::CPtr gd = MCM::metric(_id)->currentGrowthData();
    if ( gd)
    {
        rlib::RSD::CPtr rsd = gd->rsd(i);
        assert(rsd);
        const double t = std::max<double>( rsd->tmin(), std::min<double>( age, int(rsd->tmax() + 0.5)));
        zs = rsd->zscore( t, _values.at(i));
    }   // end if
    return zs;
}   // end zscore


double MetricValue::mean( double age, size_t i) const
{
    double mn = 0;
    GrowthData::CPtr gd = MCM::metric(_id)->currentGrowthData();
    if ( gd)
    {
        rlib::RSD::CPtr rsd = gd->rsd(i);
        assert(rsd);
        const double t = std::max<double>( rsd->tmin(), std::min<double>( age, int(rsd->tmax() + 0.5)));
        mn = rsd->mval( t);
    }   // end if
    return mn;
}   // end mean


void MetricValue::write( PTree& pnode, double age) const
{
    MC::Ptr mc = MCM::metric( _id);
    assert(mc);

    PTree& mnode = pnode.add("MetricValue","");

    mnode.put( "id", mc->id());
    mnode.put( "name", mc->name().toStdString());
    mnode.put( "category", mc->category().toStdString());
    const int ndps = static_cast<int>(mc->numDecimals());
    const size_t dims = mc->dims();
    mnode.put( "ndims", dims);

    GrowthData::CPtr gd = mc->currentGrowthData();

    // Output the statistics for the metric
    PTree& snode = mnode.put( "stats", "");

    if ( gd)
    {
        snode.put( "sex", FaceTools::toLongSexString( gd->sex()).toStdString());
        snode.put( "ethnicity", Ethnicities::name( gd->ethnicity()).toStdString());
        snode.put( "source", gd->source().toStdString());
    }   // end if

    for ( size_t i = 0; i < dims; ++i)
    {
        PTree& node = snode.add( "dimension", "");
        node.put( "axis", i);

        rlib::RSD::CPtr rsd;
        if ( gd)
        {
            rsd = gd->rsd(i);
            node.put( "mean", rlib::dps( rsd->mval(age), ndps));
            node.put( "stdv", rlib::dps( rsd->zval(age), ndps));
        }   // end if

        node.put( "value", rlib::dps( value(i), ndps));
        if ( rsd)
            node.put( "zscore", rlib::dps( zscore( age, i), ndps));
    }   // end for
}   // end write
