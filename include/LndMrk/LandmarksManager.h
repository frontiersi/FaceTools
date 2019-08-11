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

#ifndef FACE_TOOLS_METRIC_LANDMARKS_MANAGER_H
#define FACE_TOOLS_METRIC_LANDMARKS_MANAGER_H

#include <Landmark.h>

namespace FaceTools { namespace Landmark {

class FaceTools_EXPORT LandmarksManager
{
public:
    // Load all landmarks from the given file.
    static int load( const QString& fname);

    // Save all landmarks to the given file.
    static bool save( const std::string& fname);

    // Return the number of landmarks.
    static size_t count() { return _lmks.size();}

    // Returns alphanumerically sorted list of codes.
    static const QStringList& codes() { return _codes;}

    // Returns alphanumerically sorted list of names.
    static QStringList names();

    // Check if the given landmark name is present.
    // Lower case versions of landmark names are unique.
    static bool hasName( const QString& nm);
    static bool hasName( const std::string&);

    // Check if the given landmark code is present.
    // Landmark codes are unique and case insensitive.
    static bool hasCode( const QString& cd);
    static bool hasCode( const std::string&);

    // Returns true if name of landmark with given id was changed.
    // Landmark names cannot be changed to existing names.
    static bool changeName( int id, const QString& newname);

    // Returns Ids of all landmarks.
    static const IntSet& ids() { return _ids;}

    // Return reference to the landmark with given id or null if doesn't exist.
    static Landmark* landmark( int id) { return _lmks.count(id) > 0 ? &_lmks.at(id) : nullptr;}

    // Return reference to the landmark with given code or null if doesn't exist.
    static Landmark* landmark( const QString& cd) { return _clmks.count(cd) > 0 ? landmark(_clmks.at(cd)) : nullptr;}
    static Landmark* landmark( const std::string& cd) { return landmark(QString(cd.c_str()));}

private:
    static IntSet _ids;
    static QStringList _codes;                      // Sorted landmark codes
    static std::unordered_map<int, Landmark> _lmks; // Landmarks keyed by id
    static std::unordered_map<QString, int> _clmks; // Landmark ids keyed by code
    static QMap<QString, QString> _names;           // Lower case mapped to normal case names
};  // end class


// Standard 49 landmark codes that should be loaded by default in the following order (IDs 1-49)
static const QString AC   =   "ac";   // alar curvature point
static const QString AL   =   "al";   // alare
static const QString CH   =   "ch";   // cheilion
static const QString C    =    "c";   // columella (medial)
static const QString CPH  =  "cph";   // crista philtri
static const QString EN   =   "en";   // endocanthion
static const QString EX   =   "ex";   // exocanthion
static const QString FT   =   "ft";   // frontotemporale
static const QString G    =    "g";   // glabella (medial)
static const QString GN   =   "gn";   // gnathion (medial)
static const QString GO   =   "go";   // gonion
static const QString LI   =   "li";   // labiale inferius (medial)
static const QString LS   =   "ls";   // labiale superius (medial)
static const QString ME   =   "me";   // menton (medial)
static const QString MIO  =  "mio";   // mid-infraorbital
static const QString N    =    "n";   // nasion (medial)
static const QString MSO  =  "mso";   // mid-supraorbital
static const QString PS   =   "ps";   // palpebrale superius
static const QString PI   =   "pi";   // palpebrale inferius
static const QString PG   =   "pg";   // pogonion (medial)
static const QString PRN  =  "prn";   // pronasale (medial)
static const QString P    =    "p";   // pupil (corneal apex)
static const QString SE   =   "se";   // sellion (medial)
static const QString STO  =  "sto";   // stomion (medial)
static const QString STS  =  "sts";   // stomion superius (medial)
static const QString STI  =  "sti";   // stomion inferius (medial)
static const QString SBAL = "sbal";   // subalare
static const QString SL   =   "sl";   // sublabiale (medial)
static const QString SN   =   "sn";   // subnasale (medial)
static const QString ZY   =   "zy";   // zygion
static const QString MF   =   "mf";   // maxillofrontale
static const QString FBS  =  "fbs";   // facial border - superior (medial)
static const QString LT   =   "lt";   // lateral temporale
static const QString MMB  =  "mmb";   // mid-mandibular border
static const QString MND  =  "mnd";   // mid-nasal dorsum (medial)
static const QString SPL  =  "spl";   // supralabiale (medial)
static const QString CHK  =  "chk";   // cheekbone
static const QString M    =    "m";   // metopion (medial)
static const QString SCM  =  "scm";   // superciliare medialis
static const QString SCL  =  "scl";   // superciliare lateralis
static const QString EU   =   "eu";   // eurion
static const QString OR   =   "or";   // orbitale
static const QString RH   =   "rh";   // rhinion (medial)
static const QString SC   =   "sc";   // superciliare
static const QString T    =    "t";   // tragion
static const QString TR   =   "tr";   // trichion (medial)
static const QString V    =    "v";   // vertex (medial)
static const QString MP   =   "mp";   // mid-philtrum (medial)
static const QString MA   =   "ma";   // mid-alare

}}  // end namespaces

using LDMKS_MAN = FaceTools::Landmark::LandmarksManager;

#endif
