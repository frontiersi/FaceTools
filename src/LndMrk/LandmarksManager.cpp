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
#include <MiscFunctions.h>
#include <QMap>
#include <QDir>
#include <QDebug>
#include <rlib/FileIO.h>
#include <iostream>
#include <cassert>
using FaceTools::Landmark::LandmarksManager;
using FaceTools::Landmark::Landmark;

// Static definitions
IntSet LandmarksManager::_ids;
IntSet LandmarksManager::_mids;
QStringList LandmarksManager::_codes;
std::unordered_map<int, Landmark> LandmarksManager::_lmks;
std::unordered_map<QString, int> LandmarksManager::_clmks;
QMap<QString, QString> LandmarksManager::_names;
IntSet LandmarksManager::_mset;
IntSet LandmarksManager::_bset;
IntSet LandmarksManager::_tset;


QStringList LandmarksManager::names()
{
    QStringList nms = _names.values();
    nms.sort(Qt::CaseInsensitive);
    return nms;
}   // end names


bool LandmarksManager::anyLandmarksVisible()
{
    for ( auto &p : _lmks)
        if ( p.second.isVisible())
            return true;
    return false;
}   // end anyLandmarksVisible


bool LandmarksManager::hasName( const QString& nm) { return _names.count( nm.toLower()) > 0;}
bool LandmarksManager::hasName( const std::string& nm) { return hasName( QString::fromStdString(nm));}

bool LandmarksManager::hasCode( const QString& cd) { return _clmks.count( cd.toLower()) > 0;}
bool LandmarksManager::hasCode( const std::string& nm) { return hasCode( QString::fromStdString(nm));}

int LandmarksManager::codeId( const QString& cd) { return hasCode(cd) ? _clmks.at(cd) : -1;}
int LandmarksManager::codeId( const std::string& cd) { return hasCode(cd) ? _clmks.at(QString::fromStdString(cd)) : -1;}


int LandmarksManager::loadImages( const QString &imgsDir)
{
    QDir idir( imgsDir);
    if ( !idir.exists() || !idir.isReadable())
    {
        qWarning() << "Unable to open directory:" << imgsDir;
        return -1;
    }   // end if

    // Map filenames to the landmark code. Note that filenames start with underscore. This is because
    // of "prn.png" which is disallowed in Windows (PRN is a reserved word for files since its a device).
    QMap<QString, QFileInfo> codeImageFiles;
    const QFileInfoList filelist = idir.entryInfoList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    for ( const QFileInfo &finfo : filelist)
        codeImageFiles[finfo.baseName().toLower().mid(1)] = finfo;  // Note that filenames start with underscore

    int count = 0;
    for ( auto &p : _lmks)
    {
        Landmark &lmk = p.second;
        if ( codeImageFiles.contains( lmk.code()))
        {
            const QString fpath = codeImageFiles[lmk.code()].absoluteFilePath();
            const QImage img( fpath);
            if ( img.isNull())
                qWarning() << "Unable to load image from:" << fpath;
            else
            {
                lmk.setPixmap( QPixmap::fromImage(img));
                count++;
            }   // end else
        }   // end if
        else
            qWarning() << "No example image for landmark:" << lmk.name();
    }   // end for
    return count;
}   // end loadImages


int LandmarksManager::load( const QString& fname)
{
    QTemporaryFile* tmpfile = writeToTempFile(fname);
    if (!tmpfile)
        return 0;

    _ids.clear();
    _mids.clear();
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
    for ( int id = 0; id < nrecs; ++id)
    {
        const rlib::StringVec& recs = lines[size_t(id)];

        const QString code = QString::fromStdString( recs[0]).trimmed().toLower();
        assert( _codes.count(code) == 0);   // Duplicate codes disallowed
        const QString name = QString::fromStdString( recs[1]).trimmed();
        assert( _names.count( name.toLower()) == 0);    // Duplicate names disallowed

        Landmark& lmk = _lmks[id];

        lmk.setId( id);
        lmk.setCode( code);
        lmk.setName( name);
        lmk.setBilateral( QString::fromStdString(recs[2]).toLower() == "b");
        lmk.setSuperior( QString::fromStdString(recs[3]).toLower() == "s");
        lmk.setSynonym( recs[4].c_str());
        lmk.setDescription( recs[5].c_str());
        lmk.setVisible(true);

        _ids.insert(id);
        if ( lmk.isMedial())
            _mids.insert(id);

        _codes.append( code);
        _names[name.toLower()] = name;
        _clmks[code] = id;
        lrecs++;
    }   // end for

    _codes.sort();
    _initAlignmentSets();
    return lrecs;
}   // end load


QString LandmarksManager::makeLandmarkString( int id, FaceTools::FaceLateral lat)
{
    assert(landmark(id));
    QString lmname = landmark(id)->name();
    QString lats;
    if ( lat == FACE_LATERAL_LEFT)
        lats = " (R)";
    else if ( lat == FACE_LATERAL_RIGHT)
        lats = " (L)";
    return lmname + lats;
}   // end makeLandmarkString


bool LandmarksManager::usedForAlignment( int id)
{
    return _mset.count(id) > 0 || _bset.count(id) > 0 || _tset.count(id) > 0;
}   // end usedForAlignment


void LandmarksManager::_initAlignmentSets()
{
    _mset.insert( codeId( G));
    _mset.insert( codeId( N));
    _mset.insert( codeId( SE));
    _mset.insert( codeId( SN));
    _mset.insert( codeId( SL));
    //_mset.insert( codeId( PG));
    
    _bset.insert( codeId( CHK));
    _bset.insert( codeId(  AC));
    _bset.insert( codeId( SBAL));
    _bset.insert( codeId(  GO));
    _bset.insert( codeId( MMB));

    _tset.insert( codeId( EN));
    _tset.insert( codeId( EX));
    _tset.insert( codeId( MF));
    _tset.insert( codeId( MSO));
    _tset.insert( codeId( MIO));
    _tset.insert( codeId( FT));
    _tset.insert( codeId( LT));
    _tset.insert( codeId( ZY));
    _tset.insert( codeId( SCM));
    _tset.insert( codeId( SCL));
}   // end _initAlignmentSets
