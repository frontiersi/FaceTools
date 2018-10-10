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

#ifndef FACE_TOOLS_METRIC_HPO_TERM_MANAGER_H
#define FACE_TOOLS_METRIC_HPO_TERM_MANAGER_H

#include <HPOTerm.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT HPOTermManager
{
public:
    // Load all HPO terms from the given file.
    static int load( const QString& fname);

    // Save all HPO terms to the given file.
    static bool save( const QString& fname);

    // Return the number of HPO terms.
    static size_t count() { return _hpos.size();}

    // Returns alphanumerically sorted list names.
    static const QStringList& names() { return _names;}

    // Returns Ids of all HPO terms.
    static const IntSet& ids() { return _ids;}

    // Returns the set of HPO term Ids associated with the given metric Id (reverse lookup).
    static const IntSet& metricHPOs( int metricId);

    // Return reference to the HPO term with given id or null if doesn't exist.
    static HPOTerm* hpo( int id) { return _hpos.count(id) > 0 ? &_hpos.at(id) : nullptr;}

private:
    static IntSet _ids;
    static QStringList _names;                      // HPO names
    static std::unordered_map<int, HPOTerm> _hpos;  // HPO terms keyed by their IDs
    static std::unordered_map<int, IntSet> _mhpos;  // IDs of HPO terms keyed by metric ID
};  // end class

}}  // end namespaces

#endif
