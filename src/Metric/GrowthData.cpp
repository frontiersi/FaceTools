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

#include <Metric/GrowthData.h>
#include <Metric/MetricManager.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <sol.hpp>
#include <QSet>
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MetricValue;
using MM = FaceTools::Metric::MetricManager;


GrowthData::Ptr GrowthData::create( int mid, size_t ndims, int8_t sex, int ethn, bool inPlane)
{
    return Ptr( new GrowthData( mid, ndims, sex, ethn, inPlane), [](GrowthData* x){ delete x;});
}   // end create


// private
GrowthData::GrowthData( int mid, size_t ndims, int8_t sex, int ethn, bool inp)
    : _id(-1), _mid(mid), _sex(sex), _ethn(ethn), _n(0), _inplane(inp), _rsds(ndims)
{
}   // end ctor


void GrowthData::setSource( const QString& s)
{
    _source = s;
    if ( !_source.isEmpty() && !_source.endsWith("."))
        _source += ".";
}   // end setSource


void GrowthData::addSource( const QString& s)
{
    QString esrc = _source;

    // Remove the period at the end if it exists.
    while ( esrc.endsWith("."))
        esrc = esrc.left(esrc.size()-1);

    // Split into ; delimited tokens
    QStringList srcs = esrc.split(";");

    // Set the new set of references (trimmed)
    QSet<QString> sset;
    for ( const QString& es : srcs)
        sset.insert(es.trimmed());

    // Add the new source
    QString ns = s;
    while ( ns.endsWith("."))
        ns = ns.left(ns.size()-1);
    sset.insert(ns.trimmed());

    // Sort them
    srcs = QStringList( sset.begin(), sset.end());
    srcs.sort();

    // Finally, reset.
    _source = QString( "%1.").arg(srcs.join("; "));
}   // end addSource


void GrowthData::setNote( const QString& s)
{
    _note = s;
    if ( !_note.isEmpty() && !_note.endsWith("."))
        _note += ".";
}   // end setNote


void GrowthData::appendNote( const QString& s)
{
    QStringList notes;
    notes << _note;
    notes << s;
    _note = notes.join(" ").trimmed();
    if ( !_note.isEmpty() && !_note.endsWith("."))
        _note += ".";
}   // end appendNote


void GrowthData::setLongNote( const QString& s)
{
    _lnote = s;
    if ( !_lnote.isEmpty() && !_lnote.endsWith("."))
        _lnote += ".";
}   // end setLongNote


namespace  {

void createAgeRange( std::vector<double>& trng, double v, float tmax)
{
    trng.clear();
    while ( v < tmax)
    {
        trng.push_back(v);
        v += 1;
    }   // end while
    trng.push_back(tmax);
}   // end createAgeRange

}   // end namespace


GrowthData::Ptr GrowthData::create( const std::vector<const GrowthData*>& gds)
{
    const size_t ngds = gds.size();
    assert( ngds > 1);

    const GrowthData *g0 = gds.front();

    const bool inplane = g0->inPlane();
    const size_t nd = g0->dims();
    const int mid = g0->metricId();

    int8_t sex = g0->sex();
    int nsmps = 0;

    QStringList srcs, notes;
    QStringSet srcsSet, notesSet;

    for ( size_t i = 0; i < ngds; ++i)
    {
        const GrowthData *gi = gds.at(i);
        assert( nd == gi->dims());
        assert( mid == gi->metricId());
        assert( inplane == gi->inPlane());

        if ( gi->sex() != sex)
            sex = UNKNOWN_SEX;

        // There may be some growth data that don't have the number of samples given.
        // In this case, combining them with data that do have N given will lead to
        // a misrepresentative N so it's better to set N as unknown for the combination.
        if ( gi->n() == 0)
            nsmps = -1; // Denotes unknown number
        if ( nsmps >= 0)
            nsmps += gi->n();

        // Compile notes from different sources.
        if ( notesSet.count(gi->note()) == 0)
        {
            notes << gi->note();
            notesSet.insert( gi->note());
        }   // end if

        // Compile sources (if different).
        if ( srcsSet.count(gi->source()) == 0)
        {
            QString lsrc = gi->source();
            while ( lsrc.endsWith("."))
                lsrc = lsrc.left(lsrc.size()-1);
            srcs << lsrc;
            srcsSet.insert(gi->source());
        }   // end if
    }   // end for

    // Get the mixed ethnicity if necessary
    IntSet eset;
    for ( const GrowthData *g : gds)
        eset.insert(g->ethnicity());
    int ethn = Ethnicities::codeMix( eset); // Look for an existing viable mixture
    if ( ethn == 0) // None found so make a new mixture
    {
        ethn = Ethnicities::makeMixedCode( eset);
        assert( ethn != 0);
    }   // end if

    // Create the combination GrowthData object
    GrowthData::Ptr gc = create( mid, nd, sex, ethn, inplane);
    srcs.sort();
    gc->setSource( QString( "%1.").arg(srcs.join("; ")));
    gc->setNote( notes.join(" ").trimmed());
    gc->setN( std::max( 0, nsmps));
    // Combine the distributions over each dimension
    std::vector<rlib::RSD::Ptr>& nrsds = gc->_rsds;
    nrsds.resize(nd);

    for ( size_t d = 0; d < nd; ++d)
    {
        float tmin = -FLT_MAX;
        float tmax = FLT_MAX;
        std::vector<rlib::RSD::CPtr> drsds(ngds);   // Want the average over dimension d
        for ( size_t i = 0; i < ngds; ++i)
        {
            drsds[i] = gds.at(i)->_rsds.at(d);
            // Get the min and max range for the independent variable on this dimension.
            // Note that the combined age domain is the intersection of the age domains over all.
            tmin = std::max<float>( tmin, float(drsds[i]->tmin()));
            tmax = std::min<float>( tmax, float(drsds[i]->tmax()));
        }   // end for
        std::vector<double> trng;
        createAgeRange( trng, tmin, tmax);
        nrsds[d] = rlib::RSD::average( trng, drsds);
    }   // end for

    return gc;
}   // end create


// private
GrowthData::~GrowthData(){}


bool GrowthData::isWithinAgeRange( float age) const
{
    bool inRng = age > 0.0;
    if ( inRng)
    {
        const size_t dm = dims();
        for ( size_t i = 0; i < dm; ++i)
        {
            if ( age < rsd(i)->tmin() || age > rsd(i)->tmax())
            {
                inRng = false;
                break;
            }   // end if
        }   // end for
    }   // end if
    return inRng;
}   // end isWithinAgeRange


namespace {
static const std::string WSTR = "[WARN] FaceTools::Metric::GrowthData::load: ";

void collectDistributionData( const sol::table &data, std::vector<rlib::RSD::Ptr> &rsds)
{
    const int tdims = int(data.size());
    rsds.resize(tdims);

    for ( int j = 1; j <= tdims; ++j)
    {
        if ( !data[j].valid())
        {
            std::cerr << WSTR << "data dimension is not a table for growth data" << std::endl;
            continue;
        }   // end if

        // Collect the data points for this dimension
        Vec_3DP dvec;
        sol::table dimj = data[j];
        for ( size_t k = 1; k <= dimj.size(); ++k)
        {
            if ( !dimj[k].valid())
            {
                std::cerr << WSTR << "datapoint is not a table for growth data" << std::endl;
                continue;
            }   // end if

            sol::table dp = dimj[k];
            if ( dp.size() != 3 || !dp[1].valid() || !dp[2].valid() || !dp[3].valid())
            {
                std::cerr << WSTR << "datapoints must be numberic 3-tuples for growth data" << std::endl;
                continue;
            }   // end if

            double t = dp[1];
            double y = dp[2];
            double z = dp[3];
            dvec.push_back( {t,y,z});
        }   // end for

        rsds[size_t(j-1)] = rlib::RSD::create(dvec);
    }   // end for
}   // end collectDistributionData
}   // end namespace


bool GrowthData::load( const QString &fpath)
{
    sol::state lua;
    lua.open_libraries( sol::lib::base);

    bool loadedOkay = false;
    try
    {
        lua.script_file( fpath.toStdString());
        loadedOkay = true;
    }   // end try
    catch ( const sol::error& e)
    {
        std::cerr << WSTR << "Unable to load and execute file '" << fpath.toStdString() << "'!" << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
        loadedOkay = false;
    }   // end catch

    if ( !loadedOkay)
        return false;

    sol::table table = lua["stats"];
    if ( !table.valid())
    {
        std::cerr << WSTR << "Lua file has no global member named stats!" << std::endl;
        return false;
    }   // end if

    if ( !table["dists"].valid())
    {
        std::cerr << WSTR << "dists member is not a table!" << std::endl;
        return false;
    }   // end if

    // Get the metric to add growth data to
    const int mid = table["mid"].get_or(-1);
    MC::Ptr mc = MM::metric( mid);
    if ( !mc)
    {
        std::cerr << WSTR << "No metric with ID " << mid << " present!" << std::endl;
        return false;
    }   // end if

    sol::table dists = table["dists"]; // Read in the growth data statistics
    for ( size_t i = 1; i <= dists.size(); ++i)   // for each distribution i in the dists table
    {
        if ( !dists[i].valid())
        {
            std::cerr << WSTR << "dists member is not a table for metric " << mid << std::endl;
            continue;
        }   // end if

        sol::table dist = dists[i];

        const int nsmp = dist["nsmp"].get_or(0);        // Number of sample points
        const int ethn = dist["ethn"].get_or(0);        // Ethnicity code
        const bool inpl = dist["inpl"].get_or(true);    // In-plane measurement method
        QString sexs, srcs, note, lnote;
        if ( sol::optional<std::string> v = dist["sexs"]) sexs = v.value().c_str();
        if ( sol::optional<std::string> v = dist["srce"]) srcs = v.value().c_str();
        if ( sol::optional<std::string> v = dist["note"]) note = v.value().c_str();
        if ( sol::optional<std::string> v = dist["lnote"]) lnote = v.value().c_str();

        if ( ethn <= 1)
        {
            std::cerr << WSTR << "distribution ethnicity not specified for growth data of metric " << mid << std::endl;
            continue;
        }   // end if

        if ( sexs.isEmpty())
        {
            std::cerr << WSTR << "distribution sexs not specified for growth data of metric " << mid << std::endl;
            continue;
        }   // end if

        if ( srcs.isEmpty())
        {
            std::cerr << WSTR << "distribution source not specified for growth data of metric " << mid << std::endl;
            continue;
        }   // end if

        if ( !dist["data"].valid())
        {
            std::cerr << WSTR << "distribution data member is not a table for growth data of metric " << mid << std::endl;
            continue;
        }   // end if

        sol::table data = dist["data"];
        if ( data.size() != mc->dims())
        {
            std::cerr << WSTR << "dimensions mismatch for growth data of metric " << mid << std::endl;
            continue;
        }   // end if

        GrowthData::Ptr gd = GrowthData::create( mid, mc->dims(), fromSexString(sexs), ethn, inpl);
        gd->setSource( srcs);
        gd->setN( nsmp);
        gd->setNote( note);
        gd->setLongNote( lnote);

        std::vector<rlib::RSD::Ptr> rsds;
        collectDistributionData( data, rsds);
#ifndef NDEBUG
        if ( mid == 8 && nsmp == 472)
        {
            std::cout << "Checking an example set of stats for innercanthal width:" << std::endl;
            const rlib::RSD &rsd = *rsds[0];
            const Vec_3DP& vec = rsd.data();
            using namespace rlib;
            std::cout << vec << std::endl;
            std::cout << "Queries (age, mean, sd):" << std::endl;
            for ( int x = 0; x <= 16; ++x)
                std::cout << x << " : " << rsd.mval(x) << " : " << rsd.zval(x) << std::endl;
        }   // end if
#endif

        for ( size_t j = 0; j < mc->dims(); ++j)
            if ( rsds.at(j))
                gd->setRSD( j, rsds.at(j));

        mc->_addGrowthData( gd);
    }   // end for

    return true;
}   // end load
