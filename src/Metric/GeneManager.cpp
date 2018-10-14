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

#include <GeneManager.h>
#include <QFile>
#include <FileIO.h> // rlib
#include <iostream>
#include <cassert>
using FaceTools::Metric::GeneManager;
using FaceTools::Metric::Gene;

// Static definitions
IntSet GeneManager::_ids;
QStringList GeneManager::_codes;
std::unordered_map<int, Gene> GeneManager::_genes;


int GeneManager::load( const QString& fname)
{
    _ids.clear();
    _genes.clear();
    _codes.clear();

    std::vector<rlib::StringVec> lines;
    int nrecs = rlib::readFlatFile( fname.toStdString(), lines, IBAR, true/*skip # symbols as well as blank lines*/);
    if ( nrecs <= 0)
        return nrecs;

    int lrecs = 0;
    bool ok = false;
    for ( size_t i = 0; i < size_t(nrecs); ++i)
    {
        const rlib::StringVec& recs = lines[i];
        int id = QString(recs[0].c_str()).toInt(&ok);
        if ( !ok || id < 0 || _genes.count(id) > 0)
        {
            std::cerr << "[ERROR] FaceTools::Metric::GeneManager::load: Skipping invalid id!" << std::endl;
            continue;
        }   // end else

        QString code = recs[1].c_str();
        _genes[id] = Gene(id, code);
        _genes[id].setRemarks( recs[2].c_str());
        _ids.insert(id);
        _codes.append(code);
        lrecs++;
    }   // end for

    _codes.sort();
    return lrecs;
}   // end load


bool GeneManager::save( const QString& fname)
{
    QFile file( fname);
    if ( !file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    for ( int id : _ids)
        out << _genes.at(id) << endl;
    out << endl;   // Add EoF newline

    file.close();
    return true;
}   // end save
