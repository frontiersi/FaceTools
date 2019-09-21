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

#include <LndMrk/LandmarksManager.h>
#include <LndMrk/Landmark.h>
using FaceTools::Landmark::Landmark;
using FaceTools::Landmark::LmkList;

int FaceTools::Landmark::fromParams( const QString& prms, LmkList& ll0, LmkList& ll1)
{
    ll0.clear();
    ll1.clear();

    IntSet lids;    // Store bilateral landmarks referenced to see if we need to populate ll1 as well

    std::istringstream iss( prms.toStdString());
    std::string tok;
    while ( iss.good())
    {
        iss >> tok;
        if ( tok.empty())
            continue;

        SpecificLandmark slmk;
        slmk.lat = FACE_LATERAL_MEDIAL;
        std::string code = tok;

        if ( tok[0] == 'L')
        {
            code = tok.substr(1);
            slmk.lat = FACE_LATERAL_LEFT;
        }   // end if
        else if ( tok[0] == 'R')
        {
            code = tok.substr(1);
            slmk.lat = FACE_LATERAL_RIGHT;
        }   // end else if

        if ( !LDMKS_MAN::landmark( code))   // Invalid landmark ID!
        {
            std::cerr << "[WARNING] FaceTools::Landmark::fromParams: Unknown landmark code " << code << std::endl;
            return 0;
        }   // end if

        Landmark* lmk = LDMKS_MAN::landmark(code);
        const int id = lmk->id();
        slmk.id = id;

        if ( lmk->isBilateral())
        {
            if ( slmk.lat == FACE_LATERAL_MEDIAL)
                slmk.lat = lids.count(id) == 0 ? FACE_LATERAL_LEFT : FACE_LATERAL_RIGHT;

            if (lids.count(id) == 0)
                lids.insert(id);
            else
                lids.erase(id);
        }   // end if
        else
        {
            if ( slmk.lat != FACE_LATERAL_MEDIAL)
            {
                std::cerr << "[WARNING] FaceTools::Landmark::fromParams: Non-bilateral landmark with lateral qualifier!" << std::endl;
                slmk.lat = FACE_LATERAL_MEDIAL;
            }   // end if
        }   // end else

        ll0.push_back(slmk);
    }   // end while

    if ( !lids.empty())
    {
        for ( const auto& lmk : ll0)
        {
            auto olmk = lmk;
            if ( lmk.lat != FACE_LATERAL_MEDIAL)
                olmk.lat = lmk.lat == FACE_LATERAL_LEFT ? FACE_LATERAL_RIGHT : FACE_LATERAL_LEFT;
            ll1.push_back(olmk);
        }   // end for
    }   // end if

    int nset = 0;
    if ( !ll0.empty())
    {
        nset = 1;
        if ( !ll1.empty())
            nset = 2;
    }   // end if

    return nset;
}   // end fromParams


QString FaceTools::Landmark::toParams( const LmkList& ll)
{
    QStringList prms;
    for ( const auto& l : ll)
    {
        QString q("%1");
        if ( l.lat == FACE_LATERAL_LEFT)
            q = "L%1";
        else if ( l.lat == FACE_LATERAL_RIGHT)
            q = "R%1";
        prms.append( q.arg( LDMKS_MAN::landmark(l.id)->code().toLower()));
    }   // end for
    return prms.join(' ');
}   // end toParams


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

