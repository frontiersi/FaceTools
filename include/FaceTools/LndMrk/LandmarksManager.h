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

#ifndef FACE_TOOLS_METRIC_LANDMARKS_MANAGER_H
#define FACE_TOOLS_METRIC_LANDMARKS_MANAGER_H

#include "Landmark.h"

namespace FaceTools { namespace Landmark {

class FaceTools_EXPORT LandmarksManager
{
public:
    // Load all landmarks data from the given file and images from the given directory.
    static int load( const QString &fname); // Returns number of loaded records
    static int loadImages( const QString &imgsdir); // Returns number of loaded images

    // Return the number of landmarks.
    static size_t count() { return _lmks.size();}

    // Returns true iff any landmarks return isVisible() true.
    static bool anyLandmarksVisible();

    // Returns alphanumerically sorted list of codes.
    static const QStringList& codes() { return _codes;}

    // Returns alphanumerically sorted list of names.
    static QStringList names();

    // Construct and return the landmark name appended with (L) or (R) if left or right side given.
    static QString makeLandmarkString( int id, FaceSide=MID);

    // Check if the given landmark name is present.
    // Lower case versions of landmark names are unique.
    static bool hasName( const QString& nm);
    static bool hasName( const std::string&);

    // Check if the given landmark code is present.
    // Landmark codes are unique and case insensitive.
    static bool hasCode( const QString&);
    static bool hasCode( const std::string&);

    // Return the numeric ID for the given code or -1 if not found.
    static int codeId( const QString& cd);
    static int codeId( const std::string&);

    // Returns IDs of all landmarks.
    static const IntSet& ids() { return _ids;}

    // Returns just the IDs of the medial landmarks.
    static const IntSet& medialIds() { return _mids;}

    // Return reference to the landmark with given id or null if doesn't exist.
    static Landmark* landmark( int id) { return _lmks.count(id) > 0 ? &_lmks.at(id) : nullptr;}

    // Convenience functions to say if the given landmark is bilateral, and superior/inferior.
    static bool isBilateral( int id) { return _lmks.count(id) > 0 ? _lmks.at(id).isBilateral() : false;}
    static bool isSuperior( int id) { return _lmks.count(id) > 0 ? _lmks.at(id).isSuperior() : false;}
    static bool isInferior( int id) { return _lmks.count(id) > 0 ? _lmks.at(id).isInferior() : false;}
    static bool isLocked( int id) { return _lmks.count(id) > 0 ? _lmks.at(id).isLocked() : false;}
    static bool isVisible( int id) { return _lmks.count(id) > 0 ? _lmks.at(id).isVisible() : false;}

    // Return reference to the landmark with given code or null if doesn't exist.
    static Landmark* landmark( const QString& cd) { return _clmks.count(cd) > 0 ? landmark(_clmks.at(cd)) : nullptr;}
    static Landmark* landmark( const std::string& cd) { return landmark(QString(cd.c_str()));}

    // Alignment sets
    static const IntSet& medialAlignmentSet() { return _mset;}
    static const IntSet& bottomAlignmentSet() { return _bset;}
    static const IntSet& topAlignmentSet() { return _tset;}
    static bool usedForAlignment( int id);

private:
    static IntSet _ids;
    static IntSet _mids;
    static QStringList _codes;                      // Sorted landmark codes
    static std::unordered_map<int, Landmark> _lmks; // Landmarks keyed by id
    static std::unordered_map<QString, int> _clmks; // Landmark ids keyed by code
    static QMap<QString, QString> _names;           // Lower case mapped to normal case names
    static IntSet _mset;
    static IntSet _bset;
    static IntSet _tset;
    static void _initAlignmentSets();
};  // end class


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
static const QString LI   =   "li";   // labrale inferius (medial)
static const QString LS   =   "ls";   // labrale superius (medial)
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
static const QString STS  =  "sts";   // stomion superius (medial)
static const QString STI  =  "sti";   // stomion inferius (medial)
static const QString SBAL = "sbal";   // subalare
static const QString SL   =   "sl";   // sublabial (medial)
static const QString SN   =   "sn";   // subnasale (medial)
static const QString ZY   =   "zy";   // zygion
static const QString MF   =   "mf";   // maxillofrontale
static const QString FBS  =  "fbs";   // facial border - superior (medial)
static const QString LT   =   "lt";   // lateral temporale
static const QString MMB  =  "mmb";   // mid-mandibular border
static const QString MND  =  "mnd";   // mid-nasal dorsum (medial)
static const QString SPL  =  "spl";   // supralabial (medial)
static const QString CHK  =  "chk";   // cheekbone
static const QString M    =    "m";   // metopion (medial)
static const QString SCM  =  "scm";   // superciliare medialis
static const QString SCL  =  "scl";   // superciliare lateralis
static const QString RH   =   "rh";   // rhinion (medial)
/*
static const QString STO  =  "sto";   // stomion (medial) (NO)
static const QString EU   =   "eu";   // eurion (NO)
static const QString OR   =   "or";   // orbitale (NO)
static const QString SC   =   "sc";   // superciliare (NO)
static const QString T    =    "t";   // tragion (NO)
static const QString TR   =   "tr";   // trichion (medial) (NO)
static const QString V    =    "v";   // vertex (medial) (NO)
static const QString MP   =   "mp";   // mid-philtrum (medial) (NO)
static const QString MA   =   "ma";   // mid-alare (NO)
*/

}}  // end namespaces

#endif
