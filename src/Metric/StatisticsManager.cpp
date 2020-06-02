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

#include <Metric/StatisticsManager.h>
#include <Metric/MetricManager.h>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <iostream>
#include <cassert>
using FaceTools::Metric::StatisticsManager;


int StatisticsManager::load( const QString& dname)
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
            qWarning() << "Unable to load statistics from " << mdir.absoluteFilePath(fname);
            continue;
        }   // end if

        nloaded++;
    }   // end for

    // For all metrics, consolidate growth data.
    for ( const QString &nm : MetricManager::names())
    {
        MC::Ptr mc = MetricManager::metricForName(nm);
        std::cout << QString("Metric %1 \"%2\": ").arg( mc->id(), int(4), int(10), QChar('0')).arg(mc->name()).toStdString();

        // Warn if the metric has no growth data
        if ( !mc->growthData().empty())
        {
            mc->growthData().combineSexes();       // Combine single sex growth curve datasets
            mc->growthData().combineEthnicities(); // Make ethnic pairs for same sexes
            size_t nInPlane = 0;
            const GrowthDataSources& allgd = mc->growthData().all();

            for ( const GrowthData *gd : allgd)
                if ( gd->inPlane())
                    nInPlane++;
            const size_t nGD = allgd.size();
            std::cout << nGD << " growth data";
            if ( !mc->fixedInPlane())
            {
                std::cout << " (" << nInPlane << " in-plane)";
                if ( nInPlane == nGD)
                    std::cout << " ZERO out-of-plane!";
                else if (nInPlane == 0)
                    std::cout << " ZERO in-plane!";
            }   // end if
        }   // end if
        else
            std::cout << "stats N/A";
        std::cout << std::endl;
    }   // end for

    return nloaded;
}   // end load
