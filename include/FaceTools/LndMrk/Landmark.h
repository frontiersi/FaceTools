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

#ifndef FACE_TOOLS_LANDMARK_LANDMARK_H
#define FACE_TOOLS_LANDMARK_LANDMARK_H

#include <FaceTools/FaceTypes.h>
#include <QTextStream>
#include <QPixmap>

namespace FaceTools { namespace Landmark {

struct FaceTools_EXPORT SpecificLandmark
{
    SpecificLandmark() : id(-1), lat(MID), prop(Vec3f::Ones()) {}
    SpecificLandmark( int i, FaceSide l) : id(i), lat(l), prop(Vec3f::Ones()) {}

    // Set from a string e.g. "4Ren", "321Lex", "001Ren", "Rex", "sbal" etc returning true on success. 
    bool set( const QString&);

    int id;         // Landmark identifier
    FaceSide lat;   // The landmark lateral (always MID for uni-lateral landmarks)
    Vec3f prop;     // Proportion of this landmark to use (x,y,z) (only relevant if used in combinations)
};  // end struct

using LmkList = std::vector<SpecificLandmark>;

FaceTools_EXPORT int fromParams( const std::vector<QString>&, LmkList&);


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

    void setBilateral( bool v) { _bilateral = v;}
    bool isBilateral() const { return _bilateral;}
    bool isMedial() const { return !_bilateral;}

    void setSuperior( bool v) { _superior = v;}
    bool isSuperior() const { return _superior;}
    bool isInferior() const { return !_superior;}

    void setPixmap( const QPixmap &pm) { _pixmap = pm;}
    const QPixmap& pixmap() const { return _pixmap;}

    // Visible to user?
    void setVisible( bool v) { _visible = v;}
    bool isVisible() const { return _visible;}

    // Locked from moving (dragging) by user?
    void setLocked( bool v) { _locked = v;}
    bool isLocked() const { return _locked;}

private:
    int _id;
    bool _bilateral;
    bool _superior;
    bool _visible;
    bool _locked;
    QString _code;
    QString _name;
    QString _synonym;
    QString _descrip;
    QPixmap _pixmap;
};  // end struct

}}   // end namespaces

#endif
