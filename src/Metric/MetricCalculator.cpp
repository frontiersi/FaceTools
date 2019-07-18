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

#include <MetricCalculator.h>
#include <MetricCalculatorTypeRegistry.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <MiscFunctions.h>
#include <cassert>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <sol.hpp>
using FaceTools::Metric::MetricCalculator;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::GrowthDataSources;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MCT;
using FaceTools::FM;
using FaceTools::FaceLateral;
using FaceTools::Landmark::LmkList;
using FaceTools::Metric::GrowthDataSources;

namespace {
static const int8_t BOTH_SEXES = FaceTools::Sex::FEMALE_SEX | FaceTools::Sex::MALE_SEX;
}   // end namespace


MetricCalculator::Ptr MetricCalculator::load( const QString &fpath)
{
    sol::state lua;
    lua.open_libraries( sol::lib::base);

    MetricCalculator::Ptr mc;
    try
    {
        lua.script_file( fpath.toStdString());
        mc = Ptr( new MetricCalculator, [](MetricCalculator* d){ delete d;});
    }   // end try
    catch ( const sol::error& e)
    {
        std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: Unable to load and execute file '" << fpath.toStdString() << "'!" << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
    }   // end catch

    if ( !mc)
        return nullptr;

    auto table = lua["mc"];
    if ( !table.valid())
    {
        std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: Lua file has no global member named mc!" << std::endl;
        return nullptr;
    }   // end if

    if ( !table["stats"].valid())
    {
        std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: stats member is not a table!" << std::endl;
        return nullptr;
    }   // end if

    int id = table["id"].get_or(-1);
    int ndps = table["ndps"].get_or(-1);
    int ndims = table["dims"].get_or(0);

    QString name, desc, catg, prms;
    if ( sol::optional<std::string> v = table["name"]) name = v.value().c_str();
    if ( sol::optional<std::string> v = table["desc"]) desc = v.value().c_str();
    if ( sol::optional<std::string> v = table["catg"]) catg = v.value().c_str();
    if ( sol::optional<std::string> v = table["prms"]) prms = v.value().c_str();

    if ( id < 0 || name.isEmpty() || ndps < 0 || catg.isEmpty() || prms.isEmpty() || ndims <= 0)
    {
        std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: incomplete metric metadata!" << std::endl;
        return nullptr;
    }   // end if

    mc->_id = id;
    mc->_name = name;
    mc->_desc = desc;
    mc->_ndps = static_cast<size_t>(ndps);
    mc->_dims = static_cast<size_t>(ndims);

    // Set the landmark parameters
    if ( Landmark::fromParams( prms, mc->_lmks0, mc->_lmks1) == 0)
        return nullptr;

    MCT* mct = MetricCalculatorTypeRegistry::makeMCT( catg, id, &mc->_lmks0, &mc->_lmks1);
    if ( !mct)
    {
        std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: Invalid category or parameters for metric " << id << ":" << std::endl;
        std::cerr << "\t" << catg.toStdString() << "; " << prms.toStdString() << std::endl;
        return nullptr;
    }   // end if

    mc->_setType(mct);

    // Read in the growth data statistics
    sol::table stats = table["stats"];
    for ( size_t i = 1; i <= stats.size(); ++i)   // for each distribution i in the stats table
    {
        if ( !stats[i].valid())
        {
            std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: stats member is not a table for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        sol::table dist = stats[i];

        QString sexs, srcs, note, lnote;
        int nsmp = dist["nsmp"].get_or(0);  // Number of sample points
        int ethn = dist["ethn"].get_or(0);  // Ethnicity code
        if ( sol::optional<std::string> v = dist["sexs"]) sexs = v.value().c_str();
        if ( sol::optional<std::string> v = dist["srce"]) srcs = v.value().c_str();
        if ( sol::optional<std::string> v = dist["note"]) note = v.value().c_str();
        if ( sol::optional<std::string> v = dist["lnote"]) lnote = v.value().c_str();

        if ( ethn <= 1)
        {
            std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: distribution ethnicity not specified for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        if ( sexs.isEmpty())
        {
            std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: distribution sexs not specified for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        if ( srcs.isEmpty())
        {
            std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: distribution source not specified for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        if ( !dist["data"].valid())
        {
            std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: Distribution data member is not a table for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        sol::table data = dist["data"];
        const int tdims = int( data.size());
        if ( tdims != ndims)
        {
            std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: number of dimensions mismatch in growth data for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        // Collect the distribution data
        std::vector<rlib::RSD::Ptr> rsds(static_cast<size_t>(tdims));
        for ( int j = 1; j <= tdims; ++j)
        {
            if ( !data[j].valid())
            {
                std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: data dimension is not a table for metric " << id << "!" << std::endl;
                continue;
            }   // end if

            // Collect the data points for this dimension
            Vec_3DP dvec;
            sol::table dimj = data[j];
            for ( size_t k = 1; k <= dimj.size(); ++k)
            {
                if ( !dimj[k].valid())
                {
                    std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: distribution dimension datapoint is not a table for metric " << id << "!" << std::endl;
                    continue;
                }   // end if

                sol::table dp = dimj[k];
                if ( dp.size() != 3 || !dp[1].valid() || !dp[2].valid() || !dp[3].valid())
                {
                    std::cerr << "[WARN] FaceTools::Metric::MetricCalculator::load: distribution dimension datapoints must be numberic 3-tuples for metric " << id << "!" << std::endl;
                    continue;
                }   // end if

                double t = dp[1];
                double y = dp[2];
                double z = dp[3];
                dvec.push_back( {t,y,z});
            }   // end for

            rsds[static_cast<size_t>(j-1)] = rlib::RSD::create(dvec);
        }   // end for

        GrowthData::Ptr gd = GrowthData::create( id, static_cast<size_t>(ndims), fromSexString(sexs), ethn);
        gd->setSource( srcs);
        gd->setN( nsmp);

        if ( !note.isEmpty())
            gd->setNote( note);
        if ( !lnote.isEmpty())
            gd->setLongNote( lnote);

        for ( size_t j = 0; j < static_cast<size_t>(ndims); ++j)
        {
            if ( rsds.at(j))
                gd->setRSD( j, rsds.at(j));
        }   // end for

        mc->_addGrowthData( gd);
        if ( mc->currentGrowthData() == nullptr)
            mc->setCurrentGrowthData( gd);
    }   // end for

    // Combine single sex growth curve datasets for FEMALE_SEX | MALE_SEX
    mc->_combineGrowthDataSexes();
    // Make ethnic pairs for same sexes.
    mc->_combineGrowthDataEthnicities();

    return mc;
}   // end load


MetricCalculator::MetricCalculator()
    : _mct(nullptr), _visible(false), _id(-1), _ndps(0), _dims(0) {}


MetricCalculator::~MetricCalculator()
{
    _gdata.clear();
    if ( _mct)
        delete _mct;
}   // end dtor


void MetricCalculator::_addGrowthData( GrowthData::Ptr gd)
{
    assert(gd->metricId() == id());
    _gdata[gd->sex()][gd->ethnicity()].insert(gd);
}   // end _addGrowthData


void MetricCalculator::_combineGrowthDataSexes()
{
    // First collate all of the GrowthData objects by source, then by ethnic group code.
    // The set of GrowthData objects will then be different sexes from the same source.
    // If there exist exactly two opposite sex GrowthData objects in the set, combine them
    // into a new GrowthData object for the FEMALE_SEX | MALE_SEX combination.
    QMap<QString, std::unordered_map<int, GrowthDataSources> > srcMap;
    for ( const auto& p : _gdata)
    {
        for ( const auto& q : p.second) // q= std::pair<int, GrowthDataSources>
        {
            const int ethn = q.first;
            for ( const GrowthData::CPtr& gd : q.second)
                srcMap[gd->source()][ethn].insert(gd);
        }   // end for
    }   // end for

    QStringList sources = srcMap.keys();
    for ( const QString& src : sources)
    {
        for ( const auto& p : srcMap[src])
        {
            const GrowthDataSources& gds = p.second;
            if ( gds.size() == 2)   // Will be same ethnic code
            {
                const GrowthData::CPtr& g0 = *gds.begin();
                const GrowthData::CPtr& g1 = *(++gds.begin());
                if ( g0->sex() != BOTH_SEXES && g0->sex() != UNKNOWN_SEX
                     && g1->sex() != BOTH_SEXES && g1->sex() != UNKNOWN_SEX
                     && g0->sex() != g1->sex())
                {
                    GrowthData::Ptr gnew = GrowthData::create( {g0, g1});
                    assert(gnew);
                    _addGrowthData( gnew);
                }   // end if
            }   // end if
        }   // end for
    }   // end for
}   // end _combineGrowthDataSexes


void MetricCalculator::_combineGrowthDataEthnicities()
{
    // Call after _combineGrowthDataEthnicities to combine different sex ethnic pairs.
    // Note that this is done across sources so initial regrouping is only by sex.
    std::unordered_map<int8_t, std::vector<GrowthData::CPtr> > sexMap;
    for ( const auto& p : _gdata)
    {
        const int8_t sex = p.first;
        for ( const auto& q : p.second) // q= std::pair<int, GrowthDataSources>
        {
            for ( const GrowthData::CPtr& gd : q.second)
                sexMap[sex].push_back(gd);
        }   // end for
    }   // end for

    for ( const auto& p : sexMap)
    {
        const std::vector<GrowthData::CPtr>& gds = p.second;
        const size_t n = gds.size();
        for ( size_t i = 0; i < n; ++i)
        {
            GrowthData::CPtr gdi = gds[i];
            for ( size_t j = i+1; j < n; ++j)
            {
                GrowthData::CPtr gdj = gds[j];
                assert( gdi->sex() == gdj->sex());  // Must be same sex
                // Only combine if different ethnicities.
                if ( gdi->ethnicity() != gdj->ethnicity())
                {
                    assert( gdi->source() != gdj->source());    // Must be different sources, or duplicate GrowthData was read in!
                    GrowthData::Ptr gnew = GrowthData::create( {gdi, gdj});
                    assert(gnew);
                    _addGrowthData( gnew);
                }   // end if
            }   // end for
        }   // end for
    }   // end for
}   // end _combineGrowthDataEthnicities


void MetricCalculator::_findMatchingGrowthData( int8_t sex, int en, bool exactEth, GrowthDataSources& mgds) const
{
    if ( _gdata.count(sex) > 0)
    {
        const std::unordered_map<int, GrowthDataSources>& egds = _gdata.at(sex);
        if ( en == 0)
        {
            // If en == 0 then no ethnicity was given so get all the GrowthData sources for the given sex.
            for ( const auto& p : egds)
            {
                const GrowthDataSources& gds = p.second;
                mgds.insert( gds.begin(), gds.end());
            }   // end for
        }   // end if
        else
        {
            for ( const auto& p : egds)
            {
                // If en belongs to the keyed ethnic group ethn then all of the sources are included.
                if ( (exactEth && en == p.first) || (!exactEth && Ethnicities::belongs( p.first, en)))
                {
                    const GrowthDataSources& gds = p.second;
                    mgds.insert( gds.begin(), gds.end());
                }   // end if
            }   // end for
        }   // end else
    }   // end if
}   // end _findMatchingGrowthData


void MetricCalculator::setCompatibleSources( const FM *fm)
{
    if ( fm)
    {
        _compatible = _compatibleGrowthData(fm);
        const int ethn = Ethnicities::codeMix({fm->maternalEthnicity(), fm->paternalEthnicity()});
        const GrowthDataSources bgds = _mostCompatible( fm->sex(), ethn);
        if ( !bgds.empty())
            setCurrentGrowthData( *bgds.begin());
    }   // end if

    if ( !fm || _compatible.empty())
        _compatible = matchingGrowthData(); // All available
}   // end setCompatibleSources


GrowthDataSources MetricCalculator::matchingGrowthData( int8_t sex, int en, bool exactEth) const
{
    GrowthDataSources mgds;
    if ( sex != UNKNOWN_SEX)
        _findMatchingGrowthData( sex, en, exactEth, mgds);
    else
    {
        _findMatchingGrowthData( MALE_SEX, en, exactEth, mgds);
        _findMatchingGrowthData( FEMALE_SEX, en, exactEth, mgds);
        _findMatchingGrowthData( BOTH_SEXES, en, exactEth, mgds);
    }   // end else

    return mgds;
}   // end matchingGrowthData


GrowthDataSources MetricCalculator::_compatibleGrowthData( const FM* fm) const
{
    GrowthDataSources gds;
    if ( fm)
    {
        bool addBothSexes = false;
        // Get sex data of mother's ethnicity
        gds = matchingGrowthData( fm->sex(), fm->maternalEthnicity(), false);
        // Include combined sex data of mother's ethnicity if only single sex in model.
        if ( fm->sex() != BOTH_SEXES && fm->sex() != UNKNOWN_SEX)
        {
            addBothSexes = true;
            GrowthDataSources gds1 = matchingGrowthData( BOTH_SEXES, fm->maternalEthnicity(), false);
            gds.insert( gds1.begin(), gds1.end());
        }   // end if

        // Is father's ethnicity different?
        if ( fm->paternalEthnicity() != fm->maternalEthnicity())
        {
            // Get sex data of father's ethnicity
            GrowthDataSources gds1 = matchingGrowthData( fm->sex(), fm->paternalEthnicity(), false);
            gds.insert( gds1.begin(), gds1.end());
            // Include combined sex data of father's ethnicity if only single sex in model.
            if ( addBothSexes)
            {
                gds1 = matchingGrowthData( BOTH_SEXES, fm->paternalEthnicity(), false);
                gds.insert( gds1.begin(), gds1.end());
            }   // end if

            // Since ethnicities are different, get the mixed ethnic code data too.
            const int mcode = Ethnicities::codeMix( {fm->maternalEthnicity(), fm->paternalEthnicity()});
            gds1 = matchingGrowthData( fm->sex(), mcode, true);
            gds.insert( gds1.begin(), gds1.end());
            // And again, include combined sex data of combined ethnicity if only single sex in model.
            if ( addBothSexes)
            {
                gds1 = matchingGrowthData( BOTH_SEXES, mcode, true);
                gds.insert( gds1.begin(), gds1.end());
            }   // end if
        }   // end if
    }   // end if
    return gds;
}   // end _compatibleGrowthData


GrowthDataSources MetricCalculator::_mostCompatible( int8_t sex, int ethn) const
{
    assert( sex != UNKNOWN_SEX);
    const GrowthDataSources& mgds = _compatible;

    // Then from this compatible set, find the set of GrowthData that give the closest match to the specified demographic criteria.
    const bool ISMIX = Ethnicities::isMixed(ethn);
    GrowthDataSources bgds;
    GrowthData::CPtr bgd;
    int bmd = INT_MAX;  // Should try to minimise
    bool bmx = false;   // Whether the best match is a match on all ethnicities in the mix or not (only ever true if isMix true).
    for ( GrowthData::CPtr gd : mgds)
    {
        // If this is a mixed code, a better match is if the ethnic group of this data source
        // is a parent for all the specified ethnicities - even if it's at a higher level.
        int matchDegree = Ethnicities::parentDegree( gd->ethnicity(), ethn, ISMIX);
        bool mixMatch = ISMIX;
        if ( matchDegree < 0)
        {
            assert( ISMIX); // m can only be < 0 if this is a mixed code, otherwise it wouldn't have been compatible to begin with.
            // Wasn't able to match all the ethnicities so see if just one of the mix can be parented by this source.
            matchDegree = Ethnicities::parentDegree( gd->ethnicity(), ethn, false);
            assert( matchDegree >= 0);    // Must be >= 0 or wouldn't be compatible to begin with.
            mixMatch = false;
        }   // end if

        bool isBetterMatch = true;
        if ( bgd)
        {
            isBetterMatch = false;
            if (  ( mixMatch && !bmx)       // Prefer a complete matching of the ethnic mix to source data
               || ( mixMatch == bmx && matchDegree < bmd)   // Prefer a closer degree of ethnic matching to source data
               || ( mixMatch == bmx && matchDegree == bmd && gd->sex() != BOTH_SEXES))    // Prefer a closer match to sex for the same degree of ethnic matching
            {
                isBetterMatch = true;
                // The third case in the above disjunction allows the possibility to arise that the existing best
                // matching GrowthData has the same demographics as the current GrowthData. This can only be the
                // case however if the source reference is different. In this case, we don't clear the current
                // best match sources, but instead collect it as one of the best.
                if (!( mixMatch == bmx && matchDegree == bmd && gd->sex() == bgd->sex()))
                    bgds.clear();
            }   // end if
        }   // end if

        if ( isBetterMatch)
        {
            bgd = gd;
            bgds.insert(bgd);
            bmd = matchDegree;
            bmx = mixMatch;
        }   // end if
    }   // end for

    return bgds;
}   // end _mostCompatible


GrowthData::CPtr MetricCalculator::growthData( int8_t sex, int en, const QString& src) const
{
    GrowthData::CPtr gd;
    if ( _gdata.count(sex) > 0 && _gdata.at(sex).count(en) > 0)
    {
        const GrowthDataSources& gds = _gdata.at(sex).at(en);
        assert(!gds.empty());
        if ( src.isEmpty())
            gd = *gds.begin();
        else
        {
            for ( GrowthData::CPtr g : gds)
            {
                if ( g->source() == src)
                {
                    gd = g;
                    break;
                }   // end if
            }   // end for
        }   // end else
    }   // end if
    return gd;
}   // end growthData


bool MetricCalculator::canMeasure( const FM* fm) const
{
    return _mct && fm && _mct->canCalculate( fm, &_lmks0);
}   // end canMeasure


MetricValue MetricCalculator::_measure( const FM* fm, const LmkList& llst) const
{
    MetricValue mv( id());
    std::vector<double> dvals;
    assert(_mct);
    _mct->measure( dvals, fm, &llst); // Facial measurement at dimension i
    mv.setValues(dvals);
    return mv;
}   // end _measure


bool MetricCalculator::measure( FM* fm) const
{
    bool ok = false;
    if ( canMeasure(fm))
    {
        ok = true;
        if ( isBilateral())
        {
            fm->metricsL().set( _measure( fm, _lmks0));
            fm->metricsR().set( _measure( fm, _lmks1));
        }   // end if
        else
            fm->metrics().set( _measure( fm, _lmks0));
    }   // end if
    return ok;
}   // end measure
