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

#ifndef FACE_TOOLS_LANDMARK_LANDMARK_H
#define FACE_TOOLS_LANDMARK_LANDMARK_H

//#ifdef _WIN32
//#pragma warning( disable : 4251)
//#endif

#include <FaceTypes.h>
#include <QTextStream>

namespace FaceTools { namespace Landmark {

struct SpecificLandmark
{
    int id;
    FaceLateral lat;
};  // end struct

using LmkList = std::vector<Landmark::SpecificLandmark>;

// Read in landmark parameters from the given string returning 0 if the string
// was invalid, 1 if only the first landmark list is set (non-bilateral) or
// 2 if both landmark lists are set on return (bilateral).
int fromParams( const QString&, Landmark::LmkList&, Landmark::LmkList&);

QString toParams( const Landmark::LmkList&);


class FaceTools_EXPORT Landmark
{
public:
    Landmark();

    void setId( int id) {_id = id;}
    int id() const { return _id;}

    void setCode( const QString& c) {_code = c;}
    const QString& code() const { return _code;}

    void setName( const QString& n) {_name = n;}
    const QString& name() const { return _name;}

    void setSynonym( const QString& s) { _synonym = s;}
    const QString& synonym() const { return _synonym;}

    void setDescription( const QString& d) { _descrip = d;}
    const QString& description() const { return _descrip;}

    void setBilateral( bool b) { _bilateral = b;}
    bool isBilateral() const { return _bilateral;}

    void setDeletable( bool d) { _deletable = d;}
    bool isDeletable() const { return _deletable;}

    void cleanStrings();

private:
    int _id;
    QString _code;
    QString _name;
    QString _synonym;
    QString _descrip;
    bool _bilateral;
    bool _deletable;
};  // end struct


FaceTools_EXPORT QTextStream& operator<<( QTextStream&, const Landmark&);

}}   // end namespaces

#endif
