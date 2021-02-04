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

#include <Metric/GrowthDataRanker.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <QSet>
using FaceTools::Metric::GrowthDataRanker;
using GDS = FaceTools::Metric::GrowthDataSources;
using GD = FaceTools::Metric::GrowthData;
using FaceTools::FM;


GrowthDataRanker::GrowthDataRanker() : _gids(0) {}


void GrowthDataRanker::add( GD::Ptr gdptr)
{
    const GD *gd = gdptr.get();
    _all.insert(gd);
    _sdata[gd->sex()].insert(gd);
    _gdata[gd->sex()][gd->ethnicity()].insert(gd);
    _stats[_gids] = gd;
    gdptr->setId( _gids++);
    _allptrs.insert(gdptr); // Just to keep alive
}   // end add


std::unordered_set<int8_t> GrowthDataRanker::sexes() const
{
    std::unordered_set<int8_t> sx;
    for ( const auto &p : _sdata)
        sx.insert(p.first);
    return sx;
}   // end sexes


IntSet GrowthDataRanker::ethnicities() const
{
    IntSet eset;
    for ( const auto &p : _gdata)
        for ( const auto& q : p.second)
            eset.insert( q.first);
    return eset;
}   // end ethnicities


// Some stats from the same source are given separately for males and females.
// This function combines them. Note that this assumes that there are equal numbers
// in each age bracket for both sexes!
void GrowthDataRanker::combineSexes()
{
    // First collate all of the GrowthData objects by source, then by ethnic group code.
    // The set of GrowthData objects will then be different sexes from the same source.
    // If there exist exactly two opposite sex GrowthData objects in the set AND they have
    // both been measured in/out of plane combine them into a new GrowthData object for
    // the FEMALE_SEX | MALE_SEX combination.
    QMap<QString, std::unordered_map<int, GDS> > srcMap;
    for ( const auto& p : _gdata)
    {
        for ( const auto& q : p.second) // q= std::pair<int, GDS>
        {
            const int ethn = q.first;
            for ( const GD* gd : q.second)
                srcMap[gd->source()][ethn].insert(gd);
        }   // end for
    }   // end for

    QStringList sources = srcMap.keys();
    for ( const QString& src : sources)
    {
        for ( const auto& p : srcMap[src])
        {
            const GDS& gds = p.second;
            if ( gds.size() == 2)   // Will be same ethnic code
            {
                const GD *g0 = *gds.begin();
                const GD *g1 = *(++gds.begin());
                assert( g0->inPlane() == g1->inPlane());    // Same source should have the same type of measurement
                // If neither are UNKNOWN_SEX and they're both different sexes, merge to create a new one.
                if ( g0->sex() != UNKNOWN_SEX && g1->sex() != UNKNOWN_SEX && g0->sex() != g1->sex())
                {
                    GD::Ptr gnew = GD::create( {g0, g1});
                    assert( gnew);
                    assert( gnew->sex() == UNKNOWN_SEX);
                    add( gnew);
                }   // end if
            }   // end if
        }   // end for
    }   // end for
}   // end combineSexes

/*
// Ethnic combinations are found from across different sources
void GrowthDataRanker::combineEthnicities()
{
    std::unordered_map<int8_t, std::vector<const GD*> > sexMap;
    for ( const auto& p : _sdata)
    {
        const int8_t sex = p.first;
        for ( const GD *gd : p.second)
            sexMap[sex].push_back(gd);
    }   // end for

    for ( const auto& p : sexMap)
    {
        const std::vector<const GD*>& gds = p.second;
        const size_t n = gds.size();
        for ( size_t i = 0; i < n; ++i)
        {
            const GD *gdi = gds[i];
            for ( size_t j = i+1; j < n; ++j)
            {
                const GD *gdj = gds[j];
                assert( gdi->sex() == gdj->sex());  // Must be same sex

                // Only combine if different ethnicities AND measured in the same way
                if ( gdi->ethnicity() != gdj->ethnicity() && gdi->inPlane() == gdj->inPlane())
                {
                    GD::Ptr gnew = GD::create( {gdi, gdj});
                    assert(gnew);
                    add( gnew);
                }   // end if
            }   // end for
        }   // end for
    }   // end for
}   // end combineEthnicities
*/


#ifndef NDEBUG
#include <Metric/MetricManager.h>
#endif

// static
const GD* GrowthDataRanker::bestMatch( const GDS &gds, const FM *fm)
{
    if ( !fm)
        return nullptr;
    const int8_t sex = fm->sex();
    const int meth = fm->maternalEthnicity();
    const int peth = fm->paternalEthnicity();
    const float age = fm->age();
    return bestMatch( gds, sex, meth, peth, age);
}   // end bestMatch


// static
const GD* GrowthDataRanker::bestMatch( const GDS &gds, int8_t sex, int ethn)
{
    return bestMatch( gds, sex, ethn, ethn, FLT_MAX);
}   // end bestMatch


// static
const GD* GrowthDataRanker::bestMatch( const GDS &gds, int8_t sex, int meth, int peth, float age)
{
    static const int WORST_ETH_SCORE = 5;
    const GD *bgd = nullptr;
    int bestSexScore = 2;
    int bestEthScore = WORST_ETH_SCORE;
    int bestAgeScore = 1;

#ifndef NDEBUG
    int mid = -1;
#endif
    for ( const GD *gd : gds)
    {
#ifndef NDEBUG
        mid = gd->metricId();
#endif

        // Calculate the ethnicity match score
        const int ms = Ethnicities::parentDegree( gd->ethnicity(), meth);
        const int ps = Ethnicities::parentDegree( gd->ethnicity(), peth);
        int ethScore = std::max( ms, ps);   // Maximum to be most encompassing
        // Note here that either of ms or ps might be -1 (i.e., not be a child of the statistics)
        // but as long as one of them is, that's okay - the other will just be flagged as mismatching.
        if ( ethScore < 0)
            ethScore = WORST_ETH_SCORE;

        // Calculate the sex match score
        int sexScore = 2;
        if ( gd->sex() == sex)  // Perfect match?
            sexScore = 0;
        else if ( gd->sex() == UNKNOWN_SEX)
            sexScore = 1;

        // Calculate the age match score (in or out of range)
        const int ageScore = gd->isWithinAgeRange( age) ? 0 : 1;

        // Better iff the match score for this set of statistics is no worse in any factor
        if ( sexScore <= bestSexScore && ethScore <= bestEthScore && ageScore <= bestAgeScore)
        {
            bgd = gd;
            bestSexScore = sexScore;
            bestEthScore = ethScore;
            bestAgeScore = ageScore;
        }   // end if
    }   // end for

#ifndef NDEBUG
    if ( !bgd && mid >= 0)
    {
        const std::string mname = MetricManager::metric( mid)->name().toStdString();
        std::cerr << "WARNING: No best growth data matched against subject for metric " << mname << std::endl;
    }   // end if
#endif

    return bgd;
}   // end bestMatch


// Compatibility by sex, ethnicity, then age.
GDS GrowthDataRanker::compatible( const FM* fm) const
{
    if ( !fm)
        return _all;

    GDS gds;

    const int8_t sex = fm->sex();
    const int meth = fm->maternalEthnicity();
    const int peth = fm->paternalEthnicity();

    // Get data matching the mother's ethnicity
    if ( fm->maternalEthnicity() != 0)
    {
        GDS m = _compatible( sex, meth);
        gds.insert( m.begin(), m.end());
    }   // end if

    // Is father's ethnicity different?
    if ( peth != meth && peth != 0)
    {
        GDS m = _compatible( sex, peth);
        gds.insert( m.begin(), m.end());
        const int mcode = Ethnicities::codeMix( {meth, peth});
        if ( mcode != 0) // Since ethnicities are different, get the mixed ethnic code data too.
        {
            GDS m = _compatible( sex, mcode);
            gds.insert( m.begin(), m.end());
        }   // end if
    }   // end if
   
    // No compatible ethnicity? Then just get all stats by the subject's sex if possible.
    if ( gds.empty())
    {
        if ( _sdata.count(sex) > 0)
            gds = _sdata.at(sex);
        else if ( _sdata.count(UNKNOWN_SEX) > 0)
            gds = _sdata.at(UNKNOWN_SEX);
        else
            gds = _all;
    }   // end if

    GDS ogds;

    // Finally, check by age - just want the stats with matching age range
    const float age = fm->age();
    for ( const GD *gd : gds)
        if ( gd->isWithinAgeRange( age))
            ogds.insert(gd);

    // If there were no matching age ranges, then just use all
    if ( ogds.empty())
        ogds = gds;

    return ogds;
}   // end compatible


GDS GrowthDataRanker::compatible( int8_t sex, int ethn) const
{
    GDS gds;
    if ( _gdata.count(sex) > 0 && _gdata.at(sex).count(ethn) > 0)
        gds = _gdata.at(sex).at(ethn);
    return gds;
}   // end compatible


// static
QStringList GrowthDataRanker::sources( const GDS &gds)
{
    QStringList slst;
    QSet<QString> sset;
    for ( const GD *gd : gds)
        sset.insert( gd->source());
    slst = QStringList(sset.begin(), sset.end());
    slst.sort();
    return slst;
}   // end sources


const GD* GrowthDataRanker::matching( int8_t sex, int ethn, const QString& src) const
{
    const GDS gds = compatible(sex, ethn);
    for ( const GD *gd : gds)
        if ( gd->source() == src)
            return gd;
    return nullptr;
}   // end matching


const GD* GrowthDataRanker::stats( int gid) const
{
    return _stats.count(gid) > 0 ? _stats.at(gid) : nullptr;
}   // end stats


GDS GrowthDataRanker::_compatible( int8_t sex, int ethn) const
{
    GDS mgds;
    _compatible( sex, ethn, mgds);
    if ( sex != UNKNOWN_SEX)
        _compatible( UNKNOWN_SEX, ethn, mgds);
    return mgds;
}   // end _compatible


void GrowthDataRanker::_compatible( int8_t sex, int ethn, GDS& mgds) const
{
    if ( _sdata.count(sex) == 0)
        return;

    // If no ethnicity given (ethn == 0) get all the GrowthData sources for the given sex.
    if ( ethn == 0)
        mgds = _sdata.at(sex);
    else
    {
        for ( const GD *gd : _sdata.at(sex))
            if ( FaceTools::Ethnicities::belongs( gd->ethnicity(), ethn))
                mgds.insert( gd);
    }   // end else
}   // end _compatible
