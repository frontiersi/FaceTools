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

#ifndef FACE_TOOLS_METRIC_SYNDROME_MANAGER_H
#define FACE_TOOLS_METRIC_SYNDROME_MANAGER_H

#include <Metric/Syndrome.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT SyndromeManager
{
public:
    // Load all syndromes from the given file.
    static int load( const QString& fname);

    // Save all syndromes to the given file.
    static bool save( const QString& fname);

    // Return the number of syndromes.
    static size_t count() { return _syns.size();}

    // Returns alphanumerically sorted list of syndrome names.
    static const QStringList& names() { return _names;}

    // Returns alphanumerically sorted list of syndrome codes.
    static const QStringList& codes() { return _codes;}

    // Returns Ids of all syndromes.
    static const IntSet& ids() { return _ids;}

    // Returns the set of syndrome Ids associated with the given Gene Id.
    static const IntSet& geneSyndromes( int geneid);

    // Returns the set of syndrome Ids associated with the given HPO Id.
    static const IntSet& hpoSyndromes( int hpoid);

    // Return reference to the syndrome with given id or null if doesn't exist.
    static Syndrome* syndrome( int id) { return _syns.count(id) > 0 ? &_syns.at(id) : nullptr;}

private:
    static IntSet _ids;
    static QStringList _codes;
    static QStringList _names;
    static std::unordered_map<int, Syndrome> _syns;
    static std::unordered_map<int, IntSet> _gsyns;
    static std::unordered_map<int, IntSet> _hsyns;
};  // end class

}}  // end namespaces

#endif
