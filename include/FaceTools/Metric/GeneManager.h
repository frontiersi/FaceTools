/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_METRIC_GENE_MANAGER_H
#define FACE_TOOLS_METRIC_GENE_MANAGER_H

#include "Gene.h"

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT GeneManager
{
public:
    // Load all Genes from the given file.
    static int load( const QString& fname);

    // Save all Genes to the given file.
    static bool save( const QString& fname);

    // Return the number of Genes.
    static size_t count() { return _genes.size();}

    // Returns alphanumerically sorted list of codes.
    static const QStringList& codes() { return _codes;}

    // Returns Ids of all Genes.
    static const IntSet& ids() { return _ids;}

    // Return reference to the Gene with given id or null if doesn't exist.
    static Gene* gene( int id) { return _genes.count(id) > 0 ? &_genes.at(id) : nullptr;}

private:
    static IntSet _ids;
    static QStringList _codes;                      // Gene codes
    static std::unordered_map<int, Gene> _genes;    // Genes keyed by IDs
};  // end class

}}  // end namespaces

#endif
