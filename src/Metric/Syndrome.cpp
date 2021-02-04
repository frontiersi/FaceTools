/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Metric/Syndrome.h>
using FaceTools::Metric::Syndrome;

Syndrome::Syndrome() : _id(-1) {}


void Syndrome::cleanStrings()
{
    _code.replace( IBAR, '/');
    _name.replace( IBAR, '/');
}   // end cleanStrings


QTextStream& FaceTools::Metric::operator<<( QTextStream& os, const Syndrome& syn)
{
    Syndrome s = syn;
    s.cleanStrings();
    os << s.id() << IBAR << s.code() << IBAR << s.name() << IBAR;
    for ( int id : s.genes())
        os << id << SC;
    os << IBAR;
    for ( int id : s.hpos())
        os << id << SC;
    return os;
}   // end operator<<
