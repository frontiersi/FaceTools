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

template <typename T>
T getRecord( const PTree &n, const QString &camelCaseLabel)
{
    const std::string tok0 = camelCaseLabel.toStdString();
    const std::string tok1 = camelCaseLabel.toLower().toStdString();
    T val = T();
    if ( n.count(tok0) > 0)
        val = n.get<T>( tok0);
    else if ( n.count(tok1) > 0)
        val = n.get<T>( tok1);
    return val;
}   // end getRecord
