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

#include <Metric/PhenotypeManager.h>
#include <Metric/MetricManager.h>
#include <QFile>
#include <QDir>
#include <rlib/FileIO.h>
#include <iostream>
#include <cassert>
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;
using FaceTools::FM;

// Static definitions
IntSet PhenotypeManager::_ids;
QStringList PhenotypeManager::_names;
QStringList PhenotypeManager::_regions;
std::unordered_map<int, Phenotype::Ptr> PhenotypeManager::_hpos;
std::unordered_map<int, IntSet> PhenotypeManager::_mhpos;
std::unordered_map<QString, IntSet> PhenotypeManager::_rhpos;


namespace {
static const IntSet EMPTY_INT_SET;

using namespace FaceTools::Metric;


bool checkMissingCriteria( Phenotype::Ptr hpo)
{
    if ( hpo->objectiveCriteria().isEmpty())
    {
        std::cout << "Warning: skipping HPO term with no objective criteria "
            << QString("HP:%1 (%2)").arg( hpo->id(), int(7), int(10), QChar('0')).arg( hpo->name()).toStdString()
            << std::endl;
        return true;
    }   // end if
    return false;
}   // end checkMissingCriteria


bool checkMissingStats( Phenotype::Ptr hpo)
{
    // Warn if there are no stats for the metrics this HPO term uses in its criteria.
    IntSet noStats;
    IntSet noMids;
    for ( int mid : hpo->metrics())
    {
        if ( !MetricManager::metric(mid))
            noMids.insert(mid);
        else if ( MetricManager::metric(mid)->growthData().empty())
            noStats.insert(mid);
    }   // end for

    if ( !noMids.empty())
    {
        std::cout << "Warning: HPO term "
            << QString("HP:%1 (%2)").arg( hpo->id(), int(7), int(10), QChar('0')).arg( hpo->name()).toStdString()
            << " is missing required metrics:" << std::endl;
        for ( int mid : noMids)
            std::cout << QString( "\t -- %1").arg( mid).toStdString() << std::endl;
    }   // end if

    if ( !noStats.empty())
    {
        std::cout << "Warning: HPO term "
            << QString("HP:%1 (%2)").arg( hpo->id(), int(7), int(10), QChar('0')).arg( hpo->name()).toStdString()
            << " has no stats for its metric(s):" << std::endl;
        for ( int mid : noStats)
            std::cout << QString( "\t -- %1 (%2)").arg( MetricManager::metric(mid)->name()).arg( mid).toStdString() << std::endl;
    }   // end if

    return !noStats.empty();
}   // end checkMissingStats

}   // end namespace


const IntSet& PhenotypeManager::byMetric( int mid)
{
    if ( _mhpos.count(mid) == 0)
        return EMPTY_INT_SET;
    return _mhpos.at(mid);
}   // end byMetric


const IntSet& PhenotypeManager::byRegion( const QString &rname)
{
    if ( _rhpos.count(rname) == 0)
        return EMPTY_INT_SET;
    return _rhpos.at(rname);
}   // end byRegion


int PhenotypeManager::load( const QString& sdir)
{
    _ids.clear();
    _names.clear();
    _regions.clear();
    _hpos.clear();
    _mhpos.clear();
    _rhpos.clear();

    QDir hdir( sdir);
    if ( !hdir.exists() || !hdir.isReadable())
    {
        std::cerr << "[WARN] FaceTools::Metric::PhenotypeManager::load: Unable to open directory: " << sdir.toStdString() << std::endl;
        return -1;
    }   // end if

    const QStringList fnames = hdir.entryList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    int lrecs = 0;
    for ( const QString& fname : fnames)
    {
        Phenotype::Ptr hpo = Phenotype::load( hdir.absoluteFilePath(fname));
        if ( !hpo)
        {
            std::cerr << "[WARN] FaceTools::Metric::PhenotypeManager::load: Error loading Lua script " << fname.toStdString() << std::endl;
            continue;
        }   // end else

        // If there are no objective criteria defined, skip this HPO term.
        if ( checkMissingCriteria( hpo))
            continue;

        // Warn if there are no stats for the metrics this HPO term uses in its criteria.
        checkMissingStats( hpo);

        _ids.insert(hpo->id());
        _names.append(hpo->name());

        _hpos[hpo->id()] = hpo;
        for ( int mid : hpo->metrics())
            _mhpos[mid].insert(hpo->id());
        if ( _rhpos.count(hpo->region()) == 0)
            _regions.append(hpo->region());
        _rhpos[hpo->region()].insert(hpo->id());

        lrecs++;
    }   // end for

    _names.sort();
    _regions.sort();
    return lrecs;
}   // end load


IntSet PhenotypeManager::discover( const FM* fm, int aid)
{
    IntSet dids;
    for ( const auto& p : _hpos)
    {
        Phenotype::Ptr hpo = p.second;
        if ( hpo->isPresent(fm, aid))
            dids.insert( hpo->id());
    }   // end for
    return dids;
}   // end discover
