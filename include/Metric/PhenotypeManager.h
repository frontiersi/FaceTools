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

#ifndef FACE_TOOLS_METRIC_PHENOTYPE_MANAGER_H
#define FACE_TOOLS_METRIC_PHENOTYPE_MANAGER_H

#include <Phenotype.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT PhenotypeManager
{
public:
    // Load all HPO terms from the given directory of Lua scripts.
    static int load( const QString&);

    // Return the number of HPO terms.
    static size_t count() { return _hpos.size();}

    // Returns alphanumerically sorted list names.
    static const QStringList& names() { return _names;}

    // Returns Ids of all HPO terms.
    static const IntSet& ids() { return _ids;}

    // Returns the set of HPO term Ids associated with the given metric Id (reverse lookup).
    static const IntSet& metricPhenotypeIds( int metricId);

    // Return reference to the phenotype term with given id or null if doesn't exist.
    static Phenotype::Ptr phenotype( int id) { return _hpos.count(id) > 0 ? _hpos.at(id) : nullptr;}

    // Return a constant reference - only use if sure of id presence!
    static const Phenotype& cphenotype( int id) { return *_hpos.at(id);}

    // Discover and return the set of PhenotypicIndication IDs for the given model.
    // Note that demographic information about the model is ignored here - the only
    // consideration is if the model has the necessary measurements of the metrics
    // corresponding to each phenotypic indication.
    static IntSet discover( const FM*);

private:
    static IntSet _ids;
    static QStringList _names;                             // Phenotype names
    static std::unordered_map<int, Phenotype::Ptr> _hpos;  // Phenotype terms keyed by their IDs
    static std::unordered_map<int, IntSet> _mhpos;         // IDs of terms keyed by metric ID
};  // end class

}}  // end namespaces

#endif
