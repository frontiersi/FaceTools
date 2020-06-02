/************************************************************************
 * Copyright (C) 2018 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_METRIC_GENE_H
#define FACE_TOOLS_METRIC_GENE_H

#include <FaceTools/FaceTypes.h>
#include <QTextStream>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT Gene
{
public:
    Gene();
    Gene( int id, const QString& code);

    void setId( int id) { _id = id;}
    int id() const { return _id;}

    void setCode( const QString& s) { _code = s;}
    const QString& code() const { return _code;}

    void setRemarks( const QString& s) { _remarks = s;}
    const QString& remarks() const { return _remarks;}

    // Ensure all string fields have problematic characters replaced.
    void cleanStrings();

private:
    int _id;
    QString _code;
    QString _remarks;
};  // end class

FaceTools_EXPORT QTextStream& operator<<( QTextStream&, const Gene&);

}}   // end namespaces

#endif
