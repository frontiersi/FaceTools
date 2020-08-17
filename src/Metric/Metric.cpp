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

#include <Metric/Metric.h>
#include <Metric/MetricTypeRegistry.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <sol.hpp>
using FaceTools::Metric::Metric;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::MetricSet;
using FaceTools::FM;


Metric::Ptr Metric::load( const QString &fpath)
{
    static const std::string WSTR = "[WARN] FaceTools::Metric::Metric::load: ";
    sol::state lua;
    lua.open_libraries( sol::lib::base);

    Metric::Ptr mc;
    try
    {
        lua.script_file( fpath.toStdString());
        mc = Ptr( new Metric, [](Metric* d){ delete d;});
    }   // end try
    catch ( const sol::error& e)
    {
        std::cerr << WSTR << "Unable to load and execute file '" << fpath.toStdString() << "'!" << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
    }   // end catch

    if ( !mc)
        return nullptr;

    const sol::table table = lua["mc"];
    if ( !table.valid())
    {
        std::cerr << WSTR << "Lua file has no global member named mc!" << std::endl;
        return nullptr;
    }   // end if

    MetricParams mprms;
    mprms.id = table["id"].get_or(-1);
    if ( sol::optional<std::string> v = table["name"]) mc->_name = QString::fromStdString( v.value());
    if ( sol::optional<std::string> v = table["desc"]) mc->_desc = QString::fromStdString( v.value());
    if ( sol::optional<std::string> v = table["regn"]) mc->_regn = QString::fromStdString( v.value());
    mc->_ndps = static_cast<size_t>( table["ndps"].get_or(0));

    if ( mprms.id < 0 || mc->_name.isEmpty() || mc->_regn.isEmpty() || mc->_desc.isEmpty())
    {
        std::cerr << WSTR << "incomplete metric metadata!" << std::endl;
        return nullptr;
    }   // end if

    const sol::table prms = table["prms"];
    if ( !prms.valid())
    {
        std::cerr << WSTR << "Lua defined metric has no table named prms!" << std::endl;
        return nullptr;
    }   // end if

    QString catg;
    if ( sol::optional<std::string> v = prms["catg"]) catg = QString::fromStdString( v.value());
    if ( sol::optional<std::string> v = prms["norm"]) mprms.normal = QString::fromStdString( v.value()).toLower();
    if ( sol::optional<std::string> v = prms["rmks"]) mprms.remarks = QString::fromStdString( v.value());
    mprms.bilateral = prms["blat"].get_or(false);

    if ( catg.isEmpty() || mprms.normal.isEmpty())
    {
        std::cerr << WSTR << "incomplete metric parameters!" << std::endl;
        return nullptr;
    }   // end if

    const sol::table pnts = prms["pnts"];
    if ( !pnts.valid())
    {
        std::cerr << WSTR << "prms table of metric has no table named pnts!" << std::endl;
        return nullptr;
    }   // end if

    mprms.points.resize( pnts.size());
    for ( size_t i = 1; i <= pnts.size(); ++i) // Set landmarks defining the metric for each dimension i
    {
        if ( !pnts[i].valid())
        {
            std::cerr << WSTR << "pnts has no dimension table at position " << i << std::endl;
            return nullptr;
        }   // end if

        // The list of points for each dimension is comprised of a table for each point
        // which typically contains the code of a single landmark, but may contain the
        // codes of several landmarks.

        const sol::table dpts = pnts[i];    // List of points for dimension i
        std::vector<Landmark::LmkList> &pts = mprms.points[i-1];
        pts.resize(dpts.size());
        for ( size_t j = 1; j <= dpts.size(); ++j)   // For each point (which IS a landmark list)
        {
            const sol::table ptj = dpts[j];
            std::vector<QString> lmks( ptj.size());
            for ( size_t k = 1; k <= ptj.size(); ++k)    // For each landmark code in the point
                if ( sol::optional<std::string> v = ptj[k])
                    lmks[k-1] = QString::fromStdString( v.value());

            if ( Landmark::fromParams( lmks, pts[j-1]) <= 0)
            {
                std::cerr << WSTR << "no landmarks read in for points list!" << std::endl;
                return nullptr;
            }   // end if
        }   // end for
    }   // end for

    mc->_mct = MetricTypeRegistry::make( catg, mprms);
    if ( !mc->_mct)
    {
        std::cerr << WSTR << "Invalid metric category" << std::endl;
        return nullptr;
    }   // end if

    return mc;
}   // end load


Metric::Metric() : _visible(false), _ndps(0) {}


MetricValue Metric::_measure( const FM *fm, bool swap) const
{
    // Decide whether to take the measurement in-plane or not based
    // on the existence and value of the current growth data's (stats) in-plane value.
    std::vector<float> dvals;
    _mct->measure( dvals, fm, swap, inPlane());
    MetricValue mv( id());
    mv.setValues(dvals);
    return mv;
}   // end _measure


bool Metric::canMeasure( const FM *fm) const
{
    const Landmark::LandmarkSet& lmset = fm->currentLandmarks();
    for ( int lmid : _mct->landmarkIds())
        if ( !lmset.has(lmid))
            return false;
    return true;
}   // end canMeasure


bool Metric::_setIfMetricValueChanged( MetricSet &mset, const MetricValue &nv) const
{
    bool set = false;
    const int i = id();
    if ( !mset.hasMetric(i) || mset.metric(i) != nv)
    {
        mset.set(nv);
        set = true;
    }   // end if
    return set;
}   // end _setIfMetricValueChanged


bool Metric::measure( FM* fm) const
{
    bool changedVal = false;
    FaceAssessment::Ptr ass = fm->currentAssessment();

    if ( isBilateral())
    {
        if ( _setIfMetricValueChanged( ass->metrics(RIGHT), _measure( fm, true)))
            changedVal = true;
        if ( _setIfMetricValueChanged( ass->metrics(LEFT), _measure( fm, false)))
            changedVal = true;
    }   // end if
    else
    {
        if ( _setIfMetricValueChanged( ass->metrics(MID), _measure( fm, false)))
            changedVal = true;
    }   // end else
    return changedVal;
}   // end measure


void Metric::purge( const FM *fm) { _mct->purge(fm);}
