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

#include <HPOTerm.h>
using FaceTools::Metric::HPOTerm;

HPOTerm::HPOTerm() : _id(-1) {}


void HPOTerm::cleanStrings()
{
    _name.replace( IBAR, '/');
    _region.replace( IBAR, '/');
    for ( QString& s: _synonyms)
    {
        s.replace( IBAR, '/');
        s.replace( SC, '-');
    }   // end for
    _criteria.replace( IBAR, '/');
    _remarks.replace( IBAR, '/');
}   // end cleanStrings


QTextStream& FaceTools::Metric::operator<<( QTextStream& os, const HPOTerm& hpoin)
{
    HPOTerm hpo = hpoin;
    hpo.cleanStrings();
    os << hpo.id() << IBAR << hpo.name() << IBAR << hpo.region() << IBAR;
    for ( const QString& syn : hpo.synonyms())
        os << syn << SC;
    os << IBAR << hpo.criteria() << IBAR << hpo.remarks() << IBAR;
    for ( int id : hpo.metrics())
        os << id << SC;
    return os;
}   // end operator<<
