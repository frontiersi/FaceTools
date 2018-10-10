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

#include <Landmark.h>
using FaceTools::Landmark::Landmark;

Landmark::Landmark() : _id(-1) {}


void Landmark::cleanStrings()
{
    _code.replace( IBAR, '/');
    _name.replace( IBAR, '/');
    _synonym.replace( IBAR, '/');
    _descrip.replace( IBAR, '/');
}   // end cleanStrings


QTextStream& FaceTools::Landmark::operator<<( QTextStream& os, const Landmark& lm)
{
    Landmark lmk = lm;
    lmk.cleanStrings();
    os << lmk.id() << IBAR
       << lmk.code() << IBAR
       << lmk.name() << IBAR
       << lmk.isBilateral() << IBAR
       << lmk.synonym() << IBAR
       << lmk.description();
    return os;
}   // end operator<<

