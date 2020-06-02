/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ETHNICITIES_H
#define FACE_TOOLS_ETHNICITIES_H

#include "FaceTypes.h"
#include <QMap>

namespace FaceTools {

/**
 * Ethnic group naming is defined according to the Australian Bureau of Statistics
 * "1249.0 Australian Standard Classification of Cultural and Ethnic Groups, 2016" scheme.
 * This class provides for a queryable interface to flat file loaded data from this
 * scheme. The data can be downloaded from:
 * <www.abs.gov.au/AUSSTATS/abs@.nsf/DetailsPage/1249.02016>
 * Data must first be converted into two column text data with the 4 digit identifier
 * in the first column and the ethnic group name in the second column. The two columns
 * should be separated with a '|' symbol e.g.:
 * 2399|Western European, nec
 * The data are in category hierarchy as broad group (first digit of the identifier code),
 * narrow group (first two digits of the identifier code) and the cultural and ethnic
 * group specified by all four digits of the code.
 */
class FaceTools_EXPORT Ethnicities
{
public:
    /**
     * Load ethnic group data from the given file returning the
     * number of records read in or -1 if failed to read from file.
     */
    static int load( const QString& fname);

    /**
     * Returns sorted list in numerically ascending order of four digit codes starting at 0001.
     */
    static std::list<int> codes() { return _codes;}

    /**
     * Returns true iff the four digit code is for a broad category
     * (i.e. ending in three zeros and >= 1000).
     */
    static bool isBroad( int);

    /**
     * Returns true iff the four digit code is for a narrow category
     * (i.e. ending in two zeros and with the first two digits % 10 != 0).
     */
    static bool isNarrow( int);

    /**
     * Returns true if the code is > 0 and < 1000 for a supplemental group or is for a temporary (< 0).
     */
    static bool isMixed( int);

    /**
     * Returns the matching 4 digit code for the given name
     * (first converted to lower case) or zero if not found.
     */
    static int code( const QString&);

    /**
     * Returns the code's parent code(s) or empty set if no parent. It is possible that the given
     * code can have multiple parents if there are mixed parents too. Use the isBroad/Narrow/Mixed
     * functions to disseminate between the different parent types.
     */
    static const IntSet& parentCodes( int);

    /**
     * Returns the code's child codes or empty set if a leaf or not found.
     */
    static const IntSet& childCodes( int);

    /**
     * Returns the string name from the cultural and ethnic group category for the
     * four digit ethnic group code. or an empty string if the name doesn't exist.
     */
    static const QString& name( int);

    /**
     * Returns true iff cc has pc as a parent code at any level.
     * If allBelong = true, then if cc is itself a parent of codes (i.e. mixed)
     * AND cc is not referenced as a direct child of pc, then true will only
     * be returned if all of the mixed codes that cc contains are children of pc.
     * If allBelong is false (default) then for mixed cc, true is returned if at
     * least one of the codes belonging to cc is a child of pc.
     */
    static bool belongs( int pc, int cc, bool allBelong=false);

    /**
     * Returns how closely pc is to cc in terms of parental hierarchy.
     * -1 is returned if pc is not a parent of cc.
     * 0 is returned if pc == cc.
     * 1 is returned if pc is the immediate parent of cc.
     * 2 is returned if pc is the parent of the parent of cc.
     * Higher values denote a parental relationship even higher in the hierarchy.
     * The allBelong parameter is used in the same manner as for belongs().
     */
    static int parentDegree( int pc, int cc, bool allBelong=false);

    /**
     * Return the existing and most narrow code that is a parent of the set of given ethnicities
     * or zero if none found. If the given set has only a single member, that code is returned
     * with no further processing.
     */
    static int codeMix( const IntSet&);

    /**
     * Create and return a new code to be the parent of the given mix of codes.
     * If only one code is given, no new code is created and the same code is returned.
     * Zero is returned if the given set is empty.
     */
    static int makeMixedCode( const IntSet&);

private:
    static std::list<int> _codes;                    // All numerically ascending codes (not temporaries).
    static std::unordered_map<int, QString> _names;  // Four digit codes to all names.
    static QMap<QString, int> _lnames;               // Lower case mapping of names to codes.
    static std::unordered_map<int, IntSet> _groups;  // Code groupings parent to child
    static std::unordered_map<int, IntSet> _rgroups; // Code groupings child to parents
    static int _lt;

    static QString _makeMixedName( const IntSet&);

    static int _findSharedParent( int&, int, bool);

    static int _belongs( int, int, bool);
};  // end class

}  // end namespace

#endif
