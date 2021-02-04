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

#include <Metric/MetricManager.h>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <iostream>
#include <cassert>
using FaceTools::Metric::MetricManager;
using FaceTools::Metric::MCSet;
using FaceTools::Metric::MC;
using FaceTools::FM;

// Static definitions
IntSet MetricManager::_ids;
IntSet MetricManager::_bids;
std::unordered_map<int, MC::Ptr> MetricManager::_metrics;
std::unordered_map<int, IntSet> MetricManager::_lmMetrics;
std::unordered_map<QString, int> MetricManager::_nMetrics;
MCSet MetricManager::_mset;
MCSet MetricManager::_vmset;
QStringList MetricManager::_names;


int MetricManager::load( const QString& dname)
{
    _ids.clear();
    _bids.clear();
    _metrics.clear();
    _lmMetrics.clear();
    _nMetrics.clear();
    _mset.clear();
    _vmset.clear();
    _names.clear();

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
        MC::Ptr mc = MC::load( mdir.absoluteFilePath(fname));
        if ( !mc)
            continue;

        if ( mc->region().toLower() == "test")
            continue;

        if ( _metrics.count(mc->id()) > 0)    // Overwritting an existing Metric?
            qWarning() << "Overwriting existing Metric!";
        else
        {
            _names.append(mc->name());
            nloaded++;
        }   // end else

        _ids.insert(mc->id());
        if ( mc->isBilateral())
            _bids.insert(mc->id());
        _metrics[mc->id()] = mc;
        _nMetrics[mc->name().toLower()] = mc->id();
        _mset.insert(mc);
        if ( mc->visualiser() != nullptr)
            _vmset.insert(mc);

        // Get the landmarks used by this metric for storing metrics keyed by landmark
        IntSet lmids = mc->landmarkIds();
        for ( int lmid : lmids)
            _lmMetrics[lmid].insert(mc->id());
    }   // end for

    _names.sort();
    return nloaded;
}   // end load


const IntSet& MetricManager::metricsForLandmark( int lmid)
{
    static const IntSet EMPTY_SET;
    return _lmMetrics.count(lmid) == 0 ? EMPTY_SET : _lmMetrics.at(lmid);
}   // end metricsForLandmark


MC::Ptr MetricManager::metric( int id) { return _metrics.count(id) > 0 ? _metrics.at(id) : nullptr;}
const MC *MetricManager::cmetric( int id) { return _metrics.count(id) > 0 ? _metrics.at(id).get() : nullptr;}

MC::Ptr MetricManager::metricForName( const QString &nm)
{
    const QString lnm = nm.toLower();
    return _nMetrics.count(lnm) > 0 ? _metrics.at( _nMetrics.at(lnm)) : nullptr;
}   // end metricForName


void MetricManager::purge( const FM *fm)
{
    for ( auto &mp : _metrics)
        mp.second->_purge(fm);
}   // end purge
