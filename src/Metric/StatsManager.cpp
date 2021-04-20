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

#include <Metric/StatsManager.h>
#include <Metric/MetricManager.h>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <iostream>
#include <cassert>
using FaceTools::Metric::StatsManager;
using GD = FaceTools::Metric::GrowthData;
using MM = FaceTools::Metric::MetricManager;
using FaceTools::FM;

std::unordered_map<const FM*, std::unordered_map<int, const GD*> > StatsManager::_modelGDs;
std::unordered_map<int, const GD*> StatsManager::_metricGDs;
IntSet StatsManager::_metricDefaults;
QReadWriteLock StatsManager::_lock;


int StatsManager::load( const QString& dname)
{
    QDir mdir( dname);
    if ( !mdir.exists() || !mdir.isReadable())
    {
        qWarning() << "Unable to open directory: " << dname;
        return -1;
    }   // end if

    int nloaded = 0;
    const QStringList fnames = mdir.entryList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    for ( const QString& fname : fnames)
    {
        if ( !GrowthData::load( mdir.absoluteFilePath(fname)))
        {
            qWarning() << "Unable to load stats from " << mdir.absoluteFilePath(fname);
            continue;
        }   // end if

        nloaded++;
    }   // end for

    // For all metrics, consolidate growth data.
    for ( const QString &nm : MetricManager::names())
    {
        MC::Ptr mc = MetricManager::metricForName(nm);
#ifndef NDEBUG
        std::cerr << QString("Metric %1 \"%2\": ").arg( mc->id(), int(4), int(10), QChar('0')).arg(mc->name()).toStdString();
#endif
        if ( !mc->growthData().all().empty())
            mc->_combineGrowthDataSexes();   // Combine single sex growth curve datasets
#ifndef NDEBUG
        // Warn if the metric has no growth data
        if ( !mc->growthData().all().empty())
        {
            size_t nInPlane = 0;
            const GrowthDataSources& allgd = mc->growthData().all();

            for ( const GrowthData *gd : allgd)
                if ( gd->inPlane())
                    nInPlane++;
            const size_t nGD = allgd.size();
            std::cerr << nGD << " growth data";
            if ( !mc->fixedInPlane())
            {
                std::cerr << " (" << nInPlane << " in-plane)";
                if ( nInPlane == nGD)
                    std::cerr << " ZERO out-of-plane!";
                else if (nInPlane == 0)
                    std::cerr << " ZERO in-plane!";
            }   // end if
        }   // end if
        else
            std::cerr << "stats N/A";
        std::cerr << std::endl;
#endif
    }   // end for

    return nloaded;
}   // end load



StatsManager::RPtr StatsManager::stats( int mid, const FM *fm)
{
    if ( mid < 0)
        return nullptr;

    // No need to use lock since not updated from a different thread
    if ( !fm || usingDefaultMetricStats( mid))
        return RPtr( _metricGDs.at(mid), []( const GD*){/*no-op*/});

    if ( !_lock.tryLockForRead())
        return nullptr;

    assert( _modelGDs.count(fm) > 0);
    if ( _modelGDs.count(fm) == 0)
    {
        _lock.unlock();
        return nullptr;
    }   // end if

    assert( _modelGDs.at(fm).count(mid) > 0);
    if ( _modelGDs.at(fm).count(mid) == 0)
    {
        _lock.unlock();
        return nullptr;
    }   // end if

    const GD *gd = _modelGDs.at(fm).at(mid);
    return RPtr( gd, []( const GD*){ _lock.unlock();});
}   // end stats


void StatsManager::updateStatsForModel( const FM &fm)
{
    _lock.lockForWrite();
    const IntSet &mids = MM::ids();
    for ( int mid : mids)
    {
        const GrowthDataRanker &gdranker = MM::cmetric(mid)->growthData();
        const GrowthDataSources gds = gdranker.compatible( &fm);
        _modelGDs[&fm][mid] = gdranker.bestMatch( gds, &fm);
    }   // end for
    _lock.unlock();
}   // end updateStatsForModel


void StatsManager::purge( const FM &fm)
{
    _lock.lockForWrite();
    _modelGDs.erase(&fm);
    _lock.unlock();
}   // end purge


void StatsManager::setDefaultMetricStats( int mid, int8_t sex, int ethn, const QString &src)
{
    assert(MM::cmetric(mid));
    const GrowthDataRanker &gdranker = MM::cmetric(mid)->growthData();
    const GD *gd = gdranker.matching( sex, ethn, src);
    assert(gd); // Must exist or couldn't have been selected
    setDefaultMetricStats( mid, gd);
}   // end setDefaultMetricStats


void StatsManager::setDefaultMetricStats( int mid, const GD *gd)
{
    assert(mid >= 0);
    assert(gd);
    _metricGDs[mid] = gd;
}   // end setDefaultMetricStats


const GD *StatsManager::defaultMetricStats( int mid)
{
    return _metricGDs.count(mid) > 0 ? _metricGDs.at(mid) : nullptr;
}   // end defaultMetricStats


void StatsManager::setUseDefaultMetricStats( int mid, bool v)
{
    if ( v && mid >= 0)
        _metricDefaults.insert(mid);
    else
        _metricDefaults.erase(mid);
}   // end setUseDefaultMetricStats


bool StatsManager::usingDefaultMetricStats( int mid)
{
    return _metricDefaults.count(mid) > 0;
}   // end usingDefaultMetricStats
