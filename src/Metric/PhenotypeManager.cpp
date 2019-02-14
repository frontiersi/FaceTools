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

#include <PhenotypeManager.h>
#include <QFile>
#include <QDir>
#include <FileIO.h> // rlib
#include <iostream>
#include <cassert>
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;

// Static definitions
IntSet PhenotypeManager::_ids;
QStringList PhenotypeManager::_names;
std::unordered_map<int, Phenotype::Ptr> PhenotypeManager::_hpos;
std::unordered_map<int, IntSet> PhenotypeManager::_mhpos;


namespace {
static const IntSet EMPTY_INT_SET;
}   // end namespace


const IntSet& PhenotypeManager::metricPhenotypeIds( int mid)
{
    if ( _mhpos.count(mid) == 0)
        return EMPTY_INT_SET;
    return _mhpos.at(mid);
}   // end metricHPOs


int PhenotypeManager::load( const QString& sdir)
{
    _ids.clear();
    _hpos.clear();
    _mhpos.clear();
    _names.clear();

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

        for ( int mid : hpo->metrics())
            _mhpos[mid].insert(hpo->id());

        _hpos[hpo->id()] = hpo;
        _ids.insert(hpo->id());
        _names.append(hpo->name());
        lrecs++;
    }   // end for

    _names.sort();
    return lrecs;
}   // end load
