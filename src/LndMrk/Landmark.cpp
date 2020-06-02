/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
#include <QRegularExpression>
using FaceTools::Landmark::SpecificLandmark;
using FaceTools::Landmark::Landmark;
using FaceTools::Landmark::LmkList;
using FaceTools::Vec3f;


bool SpecificLandmark::set( const QString &tok)
{
    static const std::string ESTR = "[ERROR] FaceTools::Landmark::SpecificLandmark::set: ";

    static const QRegularExpression regexp("^([1-9]{1}|[0-9]{3})?(L|R)?([[:lower:]]+)$");
    const QRegularExpressionMatch match = regexp.match(tok);
    if ( !match.hasMatch())
    {
        std::cerr << ESTR << "token \"" << tok.toStdString() << "\" does not match regexp!" << std::endl;
        return false;
    }   // end if

    // Group 1 is the proportion of xyz
    // Group 2 is the face modifier (defaults to medial unless L or R given)
    // Group 3 is the landmark code (remaining lower case letters)
    const QString sprop = match.captured(1); // May be empty
    const QString sface = match.captured(2); // May be empty
    const std::string scode = match.captured(3).toStdString();

    const Landmark* lmk = LandmarksManager::landmark(scode);
    if ( !lmk)
    {
        std::cerr << ESTR << "invalid code: " << scode << std::endl;
        return false;
    }   // end if

    id = lmk->id();

    // Left/right modifier?
    if ( sface == "L")
        lat = FACE_LATERAL_LEFT;
    else if ( sface == "R")
        lat = FACE_LATERAL_RIGHT;
    else if ( lmk->isBilateral())
    {
        std::cerr << ESTR << "bilateral landmark not qualified (L|R)!" << std::endl;
        return false;
    }   // end else if

    if ( !lmk->isBilateral() && lat != FACE_LATERAL_MEDIAL)
    {
        std::cerr << ESTR << "Non-bilateral landmark with lateral qualifier!" << std::endl;
        return false;
    }   // end else

    // Proportion of coordinate
    if ( !sprop.isEmpty())
    {
        const int v = sprop.toInt();
        if ( sprop.length() == 1)
            prop[0] = prop[1] = prop[2] = v;
        else if ( v > 0)
        {
            prop[0] = QString(sprop.at(0)).toInt();
            prop[1] = QString(sprop.at(1)).toInt();
            prop[2] = QString(sprop.at(2)).toInt();
        }   // end else if
        else
        {
            std::cerr << ESTR << "Zero weights set for every landmark coordinate!" << std::endl;
            return false;
        }   // end else
    }   // end if

    return true;
}   // end set


int FaceTools::Landmark::fromParams( const std::vector<QString>& prms, LmkList& ll)
{
    ll.clear();

    Vec3f sums = Vec3f::Zero();
    for ( const QString &tok : prms)
    {
        SpecificLandmark slmk;
        if ( slmk.set( tok))
        {
            ll.push_back(slmk);
            sums += slmk.prop;
        }   // end if
    }   // end for

    // Adjust proportions of each landmark to set as floats
    for ( SpecificLandmark &slmk : ll)
    {
        slmk.prop[0] = slmk.prop[0] / sums[0];
        slmk.prop[1] = slmk.prop[1] / sums[1];
        slmk.prop[2] = slmk.prop[2] / sums[2];
    }   // end for

    return static_cast<int>(ll.size());
}   // end fromParams


Landmark::Landmark() : _id(-1), _bilateral(false), _superior(false), _visible(true), _locked(true) {}

