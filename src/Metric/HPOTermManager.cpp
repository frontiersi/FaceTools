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

#include <HPOTermManager.h>
#include <QFile>
#include <FileIO.h> // rlib
#include <iostream>
#include <cassert>
using FaceTools::Metric::HPOTermManager;
using FaceTools::Metric::HPOTerm;

// Static definitions
IntSet HPOTermManager::_ids;
QStringList HPOTermManager::_names;
std::unordered_map<int, HPOTerm> HPOTermManager::_hpos;
std::unordered_map<int, IntSet> HPOTermManager::_mhpos;

namespace {
static const IntSet EMPTY_INT_SET;
}   // end namespace


const IntSet& HPOTermManager::metricHPOs( int mid)
{
    if ( _mhpos.count(mid) == 0)
        return EMPTY_INT_SET;
    return _mhpos.at(mid);
}   // end metricHPOs


int HPOTermManager::load( const QString& fname)
{
    _ids.clear();
    _hpos.clear();
    _mhpos.clear();
    _names.clear();

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
        if ( !ok || id < 0 || _hpos.count(id) > 0)
        {
            std::cerr << "[WARNING] FaceTools::Metric::HPOTermManager::load: Invalid HPO id read! Problem field: '" << recs[0] << "'" << std::endl;
            continue;
        }   // end else

        HPOTerm& hpo = _hpos[id];
        hpo.setId( id);

        hpo.setName( recs[1].c_str());
        hpo.setRegion( recs[2].c_str());
        hpo.setSynonyms( QString( recs[3].c_str()).split(SC, QString::SkipEmptyParts));
        hpo.setCriteria( recs[4].c_str());
        hpo.setRemarks( recs[5].c_str());

        QStringList smids = QString( recs[6].c_str()).split(SC, QString::SkipEmptyParts);
        for ( const QString& smid : smids)
        {
            int mid = smid.toInt(&ok);
            if ( ok)
            {
                hpo.addMetric(mid);
                _mhpos[mid].insert(id);
            }   // end if
        }   // end for

        // Check that metrics set
        if ( hpo.metrics().empty())
            std::cerr << "[WARNING] FaceTools::Metric::HPOTermManager::load: No metrics set for HPO term " << id << " (invalid metric ids perhaps?)!" << std::endl;

        _ids.insert(id);
        _names.append(hpo.name());
        lrecs++;
    }   // end for

    _names.sort();
    return lrecs;
}   // end load


bool HPOTermManager::save( const QString& fname)
{
    QFile file( fname);
    if ( !file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "# Records in format:" << endl;
    out << "# ID|Name|Region|Synonym1;Synonym2;...;SynonymN|Criteria|Remarks|Metric1;Metric2;...;MetricN" << endl;
    out << "# Where the Metric component specifies the metric IDs related to the HPO term." << endl;

    // Group by region for output
    std::unordered_map<QString, std::vector<const HPOTerm*> > rhpos;
    for ( int id : _ids)
    {
        const HPOTerm* h = hpo(id);
        rhpos[h->region()].push_back(h);
    }   // end for

    // Output region groups with a blank line and comment before each group starts
    for ( const auto& p : rhpos)
    {
        const QString& region = p.first;
        out << endl << "# " << region << endl;
        const std::vector<const HPOTerm*>& rtrms = p.second;
        for ( const HPOTerm* h : rtrms)
            out << *h << endl;
    }   // end for
    out << endl;   // Add EoF newline

    file.close();
    return true;
}   // end save
