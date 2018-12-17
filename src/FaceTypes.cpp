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

#include <FaceTypes.h>
#include <FaceViewSet.h>

void FaceTools::registerTypes()
{
    qRegisterMetaType<FaceTools::Action::EventSet>("FaceTools::Action::EventSet");
    qRegisterMetaType<FaceTools::Action::EventId>("FaceTools::Action::EventId");
//    qRegisterMetaType<FaceTools::Vis::FaceView>("FaceTools::Vis::FaceView");
    qRegisterMetaType<FaceTools::FaceViewSet>("FaceTools::FaceViewSet");
}   // end registerTypes


QString FaceTools::toSexString( int8_t s)
{
    if ( s == FaceTools::UNKNOWN_SEX)
        return "U";

    std::ostringstream oss;
    if ( s & FaceTools::FEMALE_SEX)
        oss << "F ";
    if ( s & FaceTools::MALE_SEX)
        oss << "M";

    QString sstr = oss.str().c_str();
    return sstr.trimmed();
}   // end toSexString


QString FaceTools::toLongSexString( int8_t s)
{
    if ( s == FaceTools::UNKNOWN_SEX)
        return "Unknown";

    QString fstr, mstr, estr;
    if ( s & FaceTools::FEMALE_SEX)
        fstr = "Female";
    if ( s & FaceTools::MALE_SEX)
        mstr = "Male";
    if ( !mstr.isEmpty() && !fstr.isEmpty())
        estr = " | ";
    return fstr + estr + mstr;
}   // end namespace


int8_t FaceTools::fromSexString( const QString& s)
{
    int8_t sex = FaceTools::UNKNOWN_SEX;
    QStringList toks = s.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    for ( const QString& t : toks)
    {
        QString lt = t.toLower();
        if ( lt == "f")
            sex |= FaceTools::FEMALE_SEX;
        else if ( lt == "m")
            sex |= FaceTools::MALE_SEX;
    }   // end for

    return sex;
}   // end fromSexString


int8_t FaceTools::fromLongSexString( const QString& s)
{
    int8_t sex = FaceTools::UNKNOWN_SEX;
    QStringList toks = s.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    for ( const QString& t : toks)
    {
        QString lt = t.toLower();
        if ( lt == "female")
            sex |= FaceTools::FEMALE_SEX;
        else if ( lt == "male")
            sex |= FaceTools::MALE_SEX;
    }   // end for

    return sex;
}   // end fromLongSexString
