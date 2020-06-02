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

#include <Metric/MetricValue.h>
#include <Metric/MetricManager.h>
#include <Ethnicities.h>
#include <rlib/Round.h>
#include <cassert>
using FaceTools::Metric::MetricValue;
using MM = FaceTools::Metric::MetricManager;


MetricValue::MetricValue( int id) : _id(id) {}


float MetricValue::zscore( float age, size_t i) const
{
    float zs = 0;
    const GrowthData *gd = MM::metric(_id)->growthData().current();
    if ( gd)
    {
        rlib::RSD::CPtr rsd = gd->rsd(i);
        assert(rsd);
        const double t = std::max<double>( rsd->tmin(), std::min<double>( age, int(rsd->tmax() + 0.5)));
        zs = float(rsd->zscore( t, _values.at(i)));
    }   // end if
    return zs;
}   // end zscore


float MetricValue::mean( float age, size_t i) const
{
    float mn = 0;
    const GrowthData *gd = MM::metric(_id)->growthData().current();
    if ( gd)
    {
        rlib::RSD::CPtr rsd = gd->rsd(i);
        assert(rsd);
        const double t = std::max<double>( rsd->tmin(), std::min<double>( age, int(rsd->tmax() + 0.5)));
        mn = float(rsd->mval( t));
    }   // end if
    return mn;
}   // end mean


void MetricValue::write( PTree& pnode, float age) const
{
    MC::Ptr mc = MM::metric( _id);
    assert(mc);

    PTree& mnode = pnode.add("MetricValue","");

    mnode.put( "id", mc->id());
    mnode.put( "name", mc->name().toStdString());
    mnode.put( "category", mc->category().toStdString());
    const int ndps = static_cast<int>(mc->numDecimals());
    const size_t dims = mc->dims();
    mnode.put( "ndims", dims);

    const GrowthData *gd = mc->growthData().current();

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


bool FaceTools::Metric::operator==( const MetricValue &mv0, const MetricValue &mv1)
{
    return mv0.id() == mv1.id() && mv0.ndims() == mv1.ndims() && mv0.values() == mv1.values();
}   // end operator==


bool FaceTools::Metric::operator!=( const MetricValue &mv0, const MetricValue &mv1) { return !(mv0 == mv1);}
