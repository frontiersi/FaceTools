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

#include <MetricCalculatorManager.h>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <iostream>
#include <cassert>
using FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Metric::MCSet;
using FaceTools::Metric::MC;

// Static definitions
IntSet MetricCalculatorManager::_ids;
std::unordered_map<int, MC::Ptr> MetricCalculatorManager::_metrics;
MCSet MetricCalculatorManager::_mset;
MCSet MetricCalculatorManager::_vmset;
QStringList MetricCalculatorManager::_names;
QStringList MetricCalculatorManager::_ethnicities;
int MetricCalculatorManager::_cmid(-1);
int MetricCalculatorManager::_pmid(-1);


int MetricCalculatorManager::load( const QString& dname)
{
    _ids.clear();
    _metrics.clear();
    _mset.clear();
    _vmset.clear();
    _names.clear();
    _ethnicities.clear();

    QDir mdir( dname);
    if ( !mdir.exists() || !mdir.isReadable())
    {
        qWarning() << "Unable to open directory: " << dname;
        return -1;
    }   // end if

    QStringSet ethnSet;
    int nloaded = 0;
    const QStringList fnames = mdir.entryList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    for ( const QString& fname : fnames)
    {
        MC::Ptr mc = MC::load( mdir.absoluteFilePath(fname));
        if ( mc)
        {
            if ( _metrics.count(mc->id()) > 0)    // Overwritting an existing MetricCalculator?
                qWarning() << "Overwriting existing MetricCalculator!";
            else
            {
                _names.append(mc->name());

                for ( const QString& s : mc->sources())
                {
                    mc->setSource(s);
                    for ( const QString& e : mc->ethnicities())
                        ethnSet.insert( e);
                }   // end for

                nloaded++;
            }   // end else

            _ids.insert(mc->id());
            _metrics[mc->id()] = mc;
            _mset.insert(mc);
            if ( mc->visualiser() != nullptr)
                _vmset.insert(mc);
        }   // end if
    }   // end for

    _names.sort();
    for ( const QString& e : ethnSet)
        _ethnicities.append(e);
    _ethnicities.sort();

    _pmid = -1;
    _cmid = *_ids.begin();
    return nloaded;
}   // end load


MC::Ptr MetricCalculatorManager::metric( int id) { return _metrics.count(id) > 0 ? _metrics.at(id) : nullptr;}

MC::Ptr MetricCalculatorManager::currentMetric() { return _metrics.count(_cmid) > 0 ? _metrics.at(_cmid) : nullptr;}

MC::Ptr MetricCalculatorManager::previousMetric() { return _metrics.count(_pmid) > 0 ? _metrics.at(_pmid) : nullptr;}


bool MetricCalculatorManager::setCurrentMetric( int mid)
{
    if ( _cmid != mid)
    {
        assert( _metrics.count(mid) > 0);
        _pmid = _cmid;
        _cmid = mid;
        _metrics.at(mid)->setSelected();  // Cause the selected signal on the metric to fire
        return true;
    }   // end if
    return false;
}   // end setActiveMetric
