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

#ifndef FACE_TOOLS_METRIC_PHENOTYPE_MANAGER_H
#define FACE_TOOLS_METRIC_PHENOTYPE_MANAGER_H

#include "Phenotype.h"

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT PhenotypeManager
{
public:
    // Load all HPO terms from the given directory of Lua scripts.
    static int load( const QString&);

    // Return the number of HPO terms.
    static size_t size() { return _hpos.size();}

    // Returns alphanumerically sorted list names.
    static const QStringList& names() { return _names;}

    // Returns Ids of all HPO terms.
    static const IntSet& ids() { return _ids;}

    // Returns identifiers of terms by region
    static const IntSet& byRegion( const QString&);

    // Regions the anatomical regions that HPO terms associate with.
    static const QStringList& regions() { return _regions;}

    // Returns the set of HPO term Ids associated with the given metric Id (reverse lookup).
    static const IntSet& byMetric( int metricId);

    // Return reference to the phenotype term with given id or null if doesn't exist.
    static Phenotype::Ptr phenotype( int id) { return _hpos.count(id) > 0 ? _hpos.at(id) : nullptr;}

    // Discover and return the set of PhenotypicIndication IDs for the given model and
    // assessment ID. If the assessment ID is < 0, then the current assessment set on the
    // model is used. Demographic information about the model is ignored here - the only
    // consideration is if the model has the necessary measurements of the metrics
    // corresponding to each phenotypic indication.
    static IntSet discover( const FM*, int aid=-1);

private:
    static IntSet _ids;
    static QStringList _names;                             // Phenotype names
    static QStringList _regions;                           // Anatomical region names
    static std::unordered_map<int, Phenotype::Ptr> _hpos;  // Phenotype terms keyed by their IDs
    static std::unordered_map<int, IntSet> _mhpos;         // IDs of terms keyed by metric ID
    static std::unordered_map<QString, IntSet> _rhpos;     // IDs of terms keyed by region string
};  // end class

}}  // end namespaces

#endif
