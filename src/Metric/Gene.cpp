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

#include <Metric/Gene.h>
using FaceTools::Metric::Gene;

Gene::Gene() : _id(-1) {}
Gene::Gene( int id, const QString& nm) : _id(id), _code(nm) {}


void Gene::cleanStrings()
{
    _code.replace( IBAR, '/');
    _remarks.replace( IBAR, '/');
}   // end cleanStrings


QTextStream& FaceTools::Metric::operator<<( QTextStream& os, const Gene& gin)
{
    Gene g = gin;
    g.cleanStrings();
    os << g.id() << IBAR << g.code() << IBAR << g.remarks();
    return os;
}   // end operator<<
