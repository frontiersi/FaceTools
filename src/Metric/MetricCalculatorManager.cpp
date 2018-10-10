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


int MetricCalculatorManager::load( const QString& mdirname)
{
    QDir mdir( mdirname);
    if ( !mdir.exists() || !mdir.isReadable())
    {
        std::cerr << "[WARNING] FaceTools::Metric:MetricCalculatorManager::createFromFiles: "
                  << "Unable to open directory: " << mdirname.toStdString() << std::endl;
        return -1;
    }   // end if

    int nloaded = 0;
    const QStringList fnames = mdir.entryList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    for ( const QString& fname : fnames)
    {
        const std::string fpath = mdir.absoluteFilePath(fname).toStdString();
        MC::Ptr mc = MC::fromFile( fpath);
        if ( mc)
        {
            if ( _metrics.count(mc->id()) > 0)    // Overwritting an existing MetricCalculator?
            {
                std::cerr << "[WARNING] FaceTools::Metric:MetricCalculatorManager::createFromFiles: "
                    << "Overwriting existing MetricCalculator!" << std::endl;
            }   // end if
            else
            {
                _names.append(mc->name());
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
    return nloaded;
}   // end load


bool MetricCalculatorManager::save( const QString& mdirname)
{

    QDir mdir( mdirname);
    if ( !mdir.exists())
        return false;

    bool saveok = false;
    for ( MC::Ptr mc : _mset)
    {
        saveok = false;
        std::string fpath = mdir.filePath( QString("%1.txt").arg(mc->id())).toStdString();
        try
        {
            std::ofstream ofs;
            ofs.open(fpath, std::ofstream::out);
            ofs << *mc;
            ofs.close();
            saveok = true;
        }   // end try
        catch ( const std::exception& e)
        {
            std::cerr << "[ERROR] FaceTools::Metric::MetricCalculatorManager::save: "
                      << "Failed to write out metric " << mc->name().toStdString() << ": " << e.what() << std::endl;
            saveok = false;
        }   // end catch

        if ( !saveok)
            break;
    }   // end for

    return saveok;
}   // end save


MC::Ptr MetricCalculatorManager::metric( int id) { return _metrics.count(id) > 0 ? _metrics.at(id) : nullptr;}
