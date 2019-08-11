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

#include <LandmarksManager.h>
#include <MiscFunctions.h>
#include <FileIO.h> // rlib
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cassert>
using FaceTools::Landmark::LandmarksManager;
using FaceTools::Landmark::Landmark;

// Static definitions
IntSet LandmarksManager::_ids;
QStringList LandmarksManager::_codes;
std::unordered_map<int, Landmark> LandmarksManager::_lmks;
std::unordered_map<QString, int> LandmarksManager::_clmks;
QMap<QString, QString> LandmarksManager::_names;


bool LandmarksManager::changeName( int id, const QString& nm)
{
    if ( ids().count(id) == 0)
        return false;

    if ( hasName(nm))
        return false;

    _names.remove( landmark(id)->name().toLower());
    landmark(id)->setName(nm);
    _names[nm.toLower()] = nm;
    return true;
}   // end changeName


QStringList LandmarksManager::names()
{
    QStringList nms = _names.values();
    nms.sort(Qt::CaseInsensitive);
    return nms;
}   // end names


bool LandmarksManager::hasName( const QString& nm) { return _names.count(nm.toLower()) > 0;}
bool LandmarksManager::hasName( const std::string& nm) { return hasName( QString(nm.c_str()));}
bool LandmarksManager::hasCode( const QString& cd) { return _clmks.count(cd.toLower()) > 0;}
bool LandmarksManager::hasCode( const std::string& nm) { return hasCode( QString(nm.c_str()));}


int LandmarksManager::load( const QString& fname)
{
    QTemporaryFile* tmpfile = writeToTempFile(fname);
    if (!tmpfile)
        return 0;

    _ids.clear();
    _codes.clear();
    _names.clear();
    _lmks.clear();
    _clmks.clear();

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
        int id = QString(recs[0].c_str()).toInt(&ok);

        if ( !ok || id < 0 || _lmks.count(id) > 0)
        {
            std::cerr << "[ERROR] FaceTools::Landmark::LandmarksManager::load: Skipping invalid id!" << std::endl;
            continue;
        }   // end else

        const QString code = boost::algorithm::to_lower_copy(recs[1]).c_str();
        if ( _codes.count(code) > 0)
        {
            std::cerr << "[ERROR] FaceTools::Landmark::LandmarksManager::load: Skipping landmark with duplicate code!" << std::endl;
            continue;
        }   // end if

        // Get lowercase version of name to check for duplicates
        QString lname = boost::algorithm::to_lower_copy(recs[2]).c_str();
        if ( _names.count(lname) > 0)
        {
            std::cerr << "[ERROR] FaceTools::Landmark::LandmarksManager::load: Skipping landmark with duplicate name!" << std::endl;
            continue;
        }   // end if

        QString name = recs[2].c_str(); // Get original version (possibly upper case)
        Landmark& lmk = _lmks[id];

        lmk.setId( id);
        lmk.setCode( code);
        lmk.setName( name);
        lmk.setBilateral( boost::algorithm::to_lower_copy(recs[3]) == "bilateral");
        lmk.setSynonym( recs[4].c_str());
        lmk.setDescription( recs[5].c_str());
        lmk.setDeletable(false);

        _ids.insert(id);
        _codes.append( code);
        _names[lname] = name;
        _clmks[code] = id;
        lrecs++;
    }   // end for

    _codes.sort();
    return lrecs;
}   // end load


bool LandmarksManager::save( const std::string& fname)
{
    QFile file( fname.c_str());
    if ( !file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    for ( int id : _ids)
        out << _lmks.at(id) << endl;
    out << endl;   // Add EoF newline

    file.close();
    return true;
}   // end save
