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

#ifndef FACE_TOOLS_METRIC_SYNDROME_H
#define FACE_TOOLS_METRIC_SYNDROME_H

#include <FaceTypes.h>
#include <QTextStream>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT Syndrome
{
public:
    Syndrome();

    void setId( int id) { _id = id;}
    int id() const { return _id;}

    void setCode( const QString& c) { _code = c;}
    const QString& code() const { return _code;}

    void setName( const QString& nm) { _name = nm;}
    const QString& name() const { return _name;}

    const IntSet& genes() const { return _genes;}
    void addGene( int id) { _genes.insert(id);}
    void removeGene( int id) { _genes.erase(id);}

    const IntSet& hpos() const { return _hpos;}
    void addHPO( int id) { _hpos.insert(id);}
    void removeHPO( int id) { _hpos.erase(id);}

    // Ensure all string fields have problematic characters replaced.
    void cleanStrings();

private:
    int _id;
    QString _code;  // OMIM:xxxx or ORPHANET:xxxx
    QString _name;
    IntSet _genes;
    IntSet _hpos;
};  // end class

FaceTools_EXPORT QTextStream& operator<<( QTextStream&, const Syndrome&);

}}   // end namespaces

#endif
