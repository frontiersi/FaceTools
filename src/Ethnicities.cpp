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

#include <Ethnicities.h>
#include <MiscFunctions.h>
#include <rlib/FileIO.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cassert>
using FaceTools::Ethnicities;

// Static definitions
std::unordered_map<int, QString> Ethnicities::_names;
QMap<QString, int> Ethnicities::_lnames;
int Ethnicities::_lt(-100);   // decrements
std::unordered_map<int, IntSet> Ethnicities::_groups;
std::unordered_map<int, IntSet> Ethnicities::_rgroups;
std::list<int> Ethnicities::_codes;


namespace {
static const QString EMPTY_STRING = "";
static const IntSet EMPTY_INT_SET;

QString scode( int code)
{
    code = abs(code);
    const QString s = QString("%1").arg(code, 4, 10, QLatin1Char('0')); // e.g. 904 --> "0904"
    assert( s.length() == 4);
    return s;
}   // end scode

int broadCode( int code) { return scode(code).left(1).toInt() * 1000;}     // 0-9
int narrowCode( int code) { return scode(code).left(2).toInt() * 100;}

}   // end namespace


bool Ethnicities::isBroad( int code)
{
    const QString ncode = scode(code);
    return ncode.left(1).toInt() > 0 && ncode.right(3) == "000";
}   // end isBroad


bool Ethnicities::isNarrow( int code)
{
    const QString ncode = scode(code);
    return ncode.left(2).toInt() % 10 != 0 && ncode.right(2).toInt() == 0;
}   // end isNarrow


bool Ethnicities::isMixed( int code)
{
    return (code < 0) || (code > 0 && code < 1000);
}   // end isMixed


int Ethnicities::code( const QString& nm)
{
    const QString lnm = nm.trimmed().toLower();
    return _lnames.count(lnm) > 0 ? _lnames[lnm] : 0;
}   // end code


const IntSet& Ethnicities::parentCodes( int code)
{
    if ( _rgroups.count(code) == 0)
        return EMPTY_INT_SET;
    return _rgroups.at(code);
}   // end parentCodes


const IntSet& Ethnicities::childCodes( int code)
{
    if ( _groups.count(code) == 0)
        return EMPTY_INT_SET;
    return _groups.at(code);
}   // end childCodes


const QString& Ethnicities::name( int code)
{
    return _names.count(code) > 0 ? _names.at(code) : EMPTY_STRING;
}   // end name


bool Ethnicities::belongs( int pc, int cc, bool allBelong)
{
    return _belongs( pc, cc, allBelong) >= 0;
}   // end belongs


int Ethnicities::load( const QString& fname)
{
    QTemporaryFile* tmpfile = writeToTempFile(fname);
    if ( !tmpfile)
        return 0;

    _codes.clear();
    _names.clear();
    _lnames.clear();
    _groups.clear();
    _rgroups.clear();
    _lt = -100;

    const QString fpath = tmpfile->fileName();
    std::vector<rlib::StringVec> lines;
    int nrecs = rlib::readFlatFile( fpath.toStdString(), lines, IBAR, true/*skip # symbols as well as blank lines*/);
    delete tmpfile;

    if ( nrecs <= 0)
        return nrecs;

    int lrecs = 0;
    bool ok = false;
    for ( size_t i = 0; i < size_t(nrecs); ++i)
    {
        const rlib::StringVec& recs = lines[i];
        const int code = QString(recs[0].c_str()).toInt(&ok);

        if ( !ok || code < 0)
        {
            std::cerr << "[WARNING] FaceTools::Ethnicities::load: Skipping invalid code!\n";
            continue;
        }   // end if

        if ( _names.count(code) > 0)
        {
            std::cerr << "[WARNING] FaceTools::Ethnicities::load: Skipping duplicate code!\n";
            continue;
        }   // end if

        const QString name = QString(recs[1].c_str()).trimmed();
        const QString lname = name.toLower();
        if ( _lnames.count(lname) > 0)
        {
            std::cerr << "[WARNING] FaceTools::Ethnicities::load: Skipping duplicate name!\n";
            continue;
        }   // end if

        _codes.push_back(code);
        _names[code] = name;
        _lnames[name.toLower()] = code;

        if ( !isBroad(code))
        {
            const size_t nChildCodes = recs.size() - 2;  // Does this code explicitly define children?
            if ( nChildCodes > 0) // Has the child codes listed afterwards in the record
            {
                for ( size_t i = 0; i < nChildCodes; ++i)
                {
                    const int ccode = QString(recs[2+i].c_str()).toInt(&ok);
                    if ( !ok)
                    {
                        std::cerr << "[WARNING] FaceTools::Ethnicities::load: Skipping invalid mixed code child!\n";
                        continue;
                    }   // end if

                    if ( _names.count(ccode) == 0)
                    {
                        std::cerr << "[WARNING] FaceTools::Ethnicities::load: Mixed child code not yet defined in file!\n";
                        continue;
                    }   // end if

                    _stashCode( code, ccode);
                    for ( int ncc : childCodes(ccode))
                    {
                        _stashCode( code, ncc);
                        for ( int cc : childCodes( ncc))
                            _stashCode( code, cc);
                    }   // end for
                }   // end for
            }   // end if
            else
            {
                const int ncode = narrowCode(code);
                _stashCode( broadCode(code), ncode);
                if ( !isNarrow(code))   // Child code
                    _stashCode( ncode, code);
            }   // end else
        }   // end if

        lrecs++;
    }   // end for

    _codes.sort();
    return lrecs;
}   // end load


void Ethnicities::_stashCode( int parent, int child)
{
    _groups[parent].insert(child);
    _rgroups[child].insert(parent);
}   // end _stashCode


QString Ethnicities::_makeMixedName( const IntSet& ethSet)
{
    // Collect the ethnic group names
    QStringList enames;
    for ( int ec : ethSet)
    {
        if ( _names.count(ec) == 0)
            return QString();
        const QString& en = Ethnicities::name(ec);
        enames.push_back(en);
    }   // end for
    enames.sort();  // Sort in alphabetic order
    return enames.join(" | ").trimmed();
}   // end _makeMixedName


int Ethnicities::makeMixedCode( const IntSet &eset)
{
    assert( codeMix(eset) == 0);    // Don't make a new code if a viable one exists!
    QString nm = _makeMixedName(eset);
    if ( nm.isEmpty())
        return 0;

    const QString lnm = nm.toLower();
    const int emix = _lt--;
    _names[emix] = nm;
    _lnames[lnm] = emix;
    for ( int e : eset)
        _stashCode( emix, e);

    return emix;
}   // end makeMixedCode


int Ethnicities::codeMix( const IntSet& cmix)
{
    if ( cmix.empty())
        return 0;

    if ( cmix.size() == 1)
        return *cmix.begin();

    int bp = 0; // Set to be the best (most narrow) parent over all elements of cmix

    for ( int pc : cmix)
    {
        for ( int cc : cmix)
        {
            if ( _findSharedParent( pc, cc, true) < 0)  // pc not a parent of cc? then try a different parent
            {
                pc = 0;
                break;
            }   // end if
        }   // end for

        if ( pc > 0)
        {
            // If pc is a parent, and so is bp, set bp to be the better (more narrow) parent.
            if ( bp == 0)   // pc always better than nothing
                bp = pc;
            else if ( pc != bp)
            {
                // If bp is a parent of pc, then pc must be more narrow and vice versa
                if ( parentDegree( bp, pc, true) >= 0)
                    bp = pc;
#ifndef NDEBUG
                else
                    assert( parentDegree( pc, bp, true) >= 0);
#endif
            }   // end else
        }   // end if

    }   // end for

    return bp;
}   // end codeMix


int Ethnicities::parentDegree( int pc, int cc, bool allBelong)
{
    return _belongs( pc, cc, allBelong);
}   // end parentDegree


int Ethnicities::_findSharedParent( int& pc, int cc, bool allBelong)
{
    if ( pc == 0 || cc == 0)
        return -1;

    int nlvls = -1;
    IntSet pgrp;
    // If cc is not a child of pc, keep setting pc to be higher in the hierarchy
    // with the hope that eventually a parent of cc will be found
    while ( pc > 0)
    {
        nlvls = _belongs( pc, cc, allBelong);
        if ( nlvls >= 0)
            break;

        if ( pgrp.empty())
            pgrp = parentCodes(pc);
        pc = 0;
        if ( !pgrp.empty())
        {
            pc = *pgrp.begin();
            pgrp.erase(pc);
        }   // end if
    }   // end while

    return nlvls;
}   // end _findSharedParent


int Ethnicities::_belongs( int pc, int cc, bool allBelong)
{
    if ( cc == pc)
        return 0;

    int rval = -1;
    const IntSet& ccodes = childCodes(pc);
    if ( ccodes.count(cc) > 0)
        rval = 1;
    else if ( isMixed(cc))
    {
        const IntSet& mcodes = childCodes(cc);
        assert(mcodes.size() > 1);

        // If allBelong is true (and cc is mixed), all of the part codes in the
        // mix must have pc as their parent for this function to return true.
        // Otherwise, only one of them is required to be parented by pc to return true.
        if ( allBelong)
        {
            for ( int mc : mcodes)
            {
                if ( ccodes.count(mc) == 0)
                {
                    allBelong = false;
                    break;
                }   // end if
            }   // end for
            if ( allBelong)
                rval = 1;
        }   // end if
        else
        {
            for ( int mc : mcodes)
            {
                if ( ccodes.count(mc) > 0)
                {
                    rval = 1;
                    break;
                }   // end if
            }   // end for
        }   // end else
    }   // end else if
    else
    {
        for ( int hc : ccodes)
        {
            int lvl = _belongs( hc, cc, allBelong);
            if ( lvl >= 0)  // hc is parent of cc?
            {
                rval = lvl + 1;
                break;
            }   // end if
        }   // end for
    }   // end else

    return rval;
}   // end _belongs



void Ethnicities::resetComboBox( QComboBox* cb, int ecode)
{
    static const IntSet EMPTY_INT_SET;

    std::list<int> path;  // Will be path from root to leaf (ecode)
    path.push_front(ecode);

    // Progressively work backwards to the broadest category setting the discovered path:
    const IntSet* pcodes = &Ethnicities::parentCodes(ecode);
    while ( !pcodes->empty())
    {
        // Find the non-mixed parent.
        int nmp = 0;
        for ( int pcode : *pcodes)
        {
            // There's only 1 non-mixed parent per ethnic code
            if ( !Ethnicities::isMixed(pcode))
            {
                nmp = pcode;
                break;
            }   // end if
        }   // end for

        pcodes = &EMPTY_INT_SET;
        if ( nmp > 0)
        {
            path.push_front( nmp);
            // Will return empty when the code at the front is of the "broad" category.
            pcodes = &Ethnicities::parentCodes( nmp);
        }   // end if
    }   // end while

    QModelIndex midx = QModelIndex();
    cb->setRootModelIndex( midx);   // Initialise to root
    int rowIdx = 0;

    if ( ecode != 0)
    {
        for ( int ec : path)
        {
            const QString &ename = Ethnicities::name(ec);
            rowIdx = cb->findData( ename, Qt::DisplayRole);
            if ( ec != ecode)
            {
                midx = cb->model()->index( rowIdx, 0, midx);
                cb->setRootModelIndex(midx);
            }   // end if
        }   // end for
    }   // end if

    cb->setCurrentIndex( rowIdx);
}   // end resetComboBox


QTools::TreeModel* Ethnicities::createComboBoxModel()
{
    QTools::TreeModel *emodel = new QTools::TreeModel;
    QTools::TreeItem *root = emodel->setNewRoot({"Cultural and Ethnic Group"});
    root->appendChild( new QTools::TreeItem( {"Not stated"}));

    QTools::TreeItem *cbroad = nullptr; // Current broad node
    QTools::TreeItem *cnrrow = nullptr; // Current narrow node

    for ( int ethn : Ethnicities::codes())
    {
        if ( ethn <= 0)
            continue;

        const QString ename = Ethnicities::name(ethn);

        if ( Ethnicities::isMixed( ethn))
            root->appendChild( new QTools::TreeItem( {ename}));
        else if ( Ethnicities::isBroad(ethn))    // 1000, 2000, 3000 etc
            cbroad = new QTools::TreeItem( {ename}, root);
        else if ( Ethnicities::isNarrow(ethn))  // 1100, 1200, 1300 etc
        {
            assert( cbroad != nullptr);
            cnrrow = new QTools::TreeItem( {ename}, cbroad);
        }   // end else if
        else
        {
            assert( cnrrow != nullptr);
            cnrrow->appendChild( new QTools::TreeItem( {ename}));
        }   // end else
    }   // end for

    return emodel;
}   // end createComboBoxModel
