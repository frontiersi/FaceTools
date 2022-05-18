/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <FileIO/FaceModelDatabase.h>
#include <FileIO/FaceModelManager.h>
#include <QTools/FileIO.h>
#include <cassert>
using FaceTools::FileIO::FaceModelDatabase;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;

// Static definitions
const QString FaceTools::FileIO::FaceModelDatabase::NO_SUBJECT_STRING("_UNKNOWN_%1");
const QRegExp FaceTools::FileIO::FaceModelDatabase::NO_SUBJECT_REGEXP("_UNKNOWN.*$");
bool FaceTools::FileIO::FaceModelDatabase::_isInit(false);
int FaceTools::FileIO::FaceModelDatabase::_imageId(0);
int FaceTools::FileIO::FaceModelDatabase::_sbjctId(0);

namespace {

// Find the image ID (pkey) from the absolute path.
int _imageKeyFromFilePath( const QString &abspath)
{
    if ( abspath.isEmpty())
        return -1;
    QSqlQuery q( QString("SELECT id FROM images WHERE filepath = '%1'").arg( abspath));
    return q.next() ? q.value(0).toInt() : -1;
}   // end _imageKeyFromFilePath


// Returns the primary key from the subjects table for the record
// with the given string identifier or -1 if no record was found.
int _subjectKeyFromIdentifier( const QString &subjectId)
{
    if ( subjectId.isEmpty())
        return -1;
    QSqlQuery q( QString("SELECT id FROM subjects WHERE identifier = '%1'").arg(subjectId));
    return q.next() ? q.value(0).toInt() : -1;
}   // end _subjectKeyFromIdentifier


// Return the current subject key associated with the image.
int _subjectKeyFromImagePath( const QString &abspath)
{
    if ( abspath.isEmpty())
        return -1;
    QSqlQuery q( QString("SELECT subject FROM images WHERE filepath = '%1'").arg( abspath));
    return q.next() ? q.value(0).toInt() : -1;
}   // end _subjectKeyFromImagePath


// Returns the subject identifier from the subjects table for the record
// with the given id or "" if no record was found.
QString _subjectIdentifierFromKey( int sid)
{
    QSqlQuery q( QString("SELECT identifier FROM subjects WHERE id = %1").arg(sid));
    return q.next() ? q.value(0).toString() : "";
}   // end _subjectIdentiferFromKey


size_t _numImagesWithSubjectKey( int sid)
{
    QSqlQuery q( QString("SELECT COUNT(*) FROM images WHERE subject = %1").arg(sid));
    return q.next() ? q.value(0).toInt() : 0;
}   // end _numImagesWithSubjectKey


bool _removeSubject( int sid) { return QSqlQuery().exec(QString("DELETE FROM subjects WHERE id = %1").arg(sid));}
bool _removeImage( int iid) { return QSqlQuery().exec(QString("DELETE FROM images WHERE id = %1").arg(iid));}


const auto IMAGES_SQL = QLatin1String(R"(
    CREATE TABLE images( id INTEGER PRIMARY KEY,
                   filepath VARCHAR NOT NULL UNIQUE,
                    imageid VARCHAR,
                  thumbnail BLOB,
                capturedate DATE,
                    subject INTEGER,
                     source VARCHAR,
                      study VARCHAR))");


const auto SUBJECTS_SQL = QLatin1String(R"(
    CREATE TABLE subjects( id INTEGER PRIMARY KEY,
                   identifier VARCHAR UNIQUE,
                    birthdate DATE,
                          sex INTEGER,
            maternalethnicity INTEGER,
            paternalethnicity INTEGER))");


QSqlRelationalTableModel* _createRelationalModel()
{
    QSqlRelationalTableModel *model = new QSqlRelationalTableModel;
    model->setTable("images");
    const int subjectIdx = model->fieldIndex("subject");
    model->setRelation( subjectIdx, QSqlRelation("subjects", "id", "identifier"));
    model->setHeaderData( subjectIdx, Qt::Horizontal, QObject::tr("Subject"));
    model->setHeaderData( model->fieldIndex("filepath"), Qt::Horizontal, QObject::tr("File Path"));
    model->setHeaderData( model->fieldIndex("source"), Qt::Horizontal, QObject::tr("Source"));
    model->setHeaderData( model->fieldIndex("study"), Qt::Horizontal, QObject::tr("Study"));
    const int capDateIdx = model->fieldIndex("capturedate");
    model->setHeaderData( capDateIdx, Qt::Horizontal, Qt::AlignHCenter, Qt::TextAlignmentRole);
    model->setHeaderData( capDateIdx, Qt::Horizontal, QObject::tr("Image Date"));
    return model;
}   // end _createRelationalModel


// Returns true iff data about the associated subject was updated.
void _updateSubject( int sid, FM &fm, bool subjectMetaAuth)
{
    if ( subjectMetaAuth)    // Update the DB with model data?
    {
        QSqlQuery q;
        if ( !q.prepare( QString("UPDATE subjects SET birthdate = ?, maternalethnicity = ?, paternalethnicity = ?, sex = ? WHERE id = %1").arg(sid)))
            std::cerr << "[ERR] FaceTools::FileIO::FaceModelDatabase::_updateSubject: Unable to prepare update!\n";
        else
        {
            q.addBindValue( fm.dateOfBirth());
            q.addBindValue( fm.maternalEthnicity());
            q.addBindValue( fm.paternalEthnicity());
            q.addBindValue( fm.sex());
            if ( !q.exec())
                std::cerr << "[ERR] FaceTools::FileIO::FaceModelDatabase::_updateSubject: Unable to update subject!\n";
        }   // end else
    }   // end if
    else    // Update the model from the DB
    {
        QSqlQuery q( QString("SELECT birthdate, maternalethnicity, paternalethnicity, sex FROM subjects WHERE id = %1").arg(sid));
        if ( q.next())
        {
            const QDate oldDOB = fm.dateOfBirth();
            const int oldmat = fm.maternalEthnicity();
            const int oldpat = fm.paternalEthnicity();
            const int8_t oldsex = fm.sex();
            const QDate dob = q.value(0).toDate();
            const int meth = q.value(1).toInt();
            const int peth = q.value(2).toInt();
            const int8_t sex = q.value(3).toInt();
            fm.setDateOfBirth( dob);
            fm.setMaternalEthnicity( meth);
            fm.setPaternalEthnicity( peth);
            fm.setSex( sex);
            fm.setMetaSaved( oldDOB == dob
                          && oldmat == meth
                          && oldpat == peth
                          && oldsex == sex);
        }   // end if
    }   // end else
}   // end _updateSubject

}   // end namespace


bool FaceModelDatabase::_refreshImage( const QString &absFilePath, FM &fm, bool subjectMetaAuth)
{
    int iid = _imageKeyFromFilePath( absFilePath);       // Image already in DB according to file path?
    const bool isUnknownSubject = fm.subjectId().isEmpty() || NO_SUBJECT_REGEXP.exactMatch( fm.subjectId());
    int sid = _subjectKeyFromIdentifier( fm.subjectId());  // Will be -1 if subjectId is empty or subject not yet in DB
    const int csid = _subjectKeyFromImagePath( absFilePath); // Existing subject key in database
    // Existing subject can't be different from the original for this image if unknown subject!
    if ( csid != sid && isUnknownSubject)
    {
        fm.setSubjectId( _subjectIdentifierFromKey( csid));  // Reset with old identifier
        sid = csid;
    }   // end if

    // If the subject matched unknown and the image is not already in the database then the subject
    // identifier is set empty. This accounts for the possibility that a 3DF contains the unknown string
    // (shouldn't happen because subject identifiers matching unknown string are removed upon save, but
    // it's possible to edit the 3DF metadata externally so need to check here too).
    if ( isUnknownSubject && iid < 0)
    {
        fm.setSubjectId("");
        sid = -1;
    }   // end if

    if ( sid < 0)   // Add new subject
    {
        sid = _sbjctId++; // Primary key for new subject
        if ( fm.subjectId().isEmpty())
            fm.setSubjectId( NO_SUBJECT_STRING.arg(sid));
        QSqlQuery q;
        q.prepare( "INSERT INTO subjects( id, identifier, birthdate, sex, maternalethnicity, paternalethnicity) VALUES( ?, ?, ?, ?, ?, ?)");
        q.addBindValue( sid);
        q.addBindValue( fm.subjectId());    // Human string identifier (not DB)
        q.addBindValue( fm.dateOfBirth());
        q.addBindValue( fm.sex());
        q.addBindValue( fm.maternalEthnicity());
        q.addBindValue( fm.paternalEthnicity());
        const bool okay = q.exec();
        assert(okay);
        if ( !okay)
            std::cerr << "[ERR] FaceTools::FileIO::FaceModelDatabase::refreshImage: INSERT subject failed!\n";
    }   // end if
    else if ( !isUnknownSubject) // Update subject in DB (subjectMetaAuth=true), or in model from DB (subjectMetaAuth=false) if valid subject identifier
        _updateSubject( sid, fm, subjectMetaAuth);

    // Write the thumbnail as a byte array for binary large object (BLOB)
    QByteArray thumbnail;
    QBuffer inBuffer( &thumbnail);
    inBuffer.open( QIODevice::WriteOnly);
    fm.thumbnail().save( &inBuffer, "PNG");

    assert( sid >= 0);
    bool newImage = false;
    if ( iid >= 0) // Existing image
    {
        QSqlTableModel table;
        table.setTable("images");
        table.setEditStrategy( QSqlTableModel::OnManualSubmit);
        table.setFilter( QString("id = %1").arg( iid));
        table.select();
        assert( table.rowCount() == 1);
        QSqlRecord record = table.record(0);

        // Is the subject key different to that stored against the image? If so, the subject
        // was changed and we need to check if the old subject is still referenced by any
        // other image. If not, we can safely remove the old subject record from the subjects table.
        const int csid = record.value("subject").toInt();
        if ( sid != csid && _numImagesWithSubjectKey( csid) == 1)    // i.e., just the current record
            _removeSubject( csid);

        record.setValue("imageid", fm.imageId());
        record.setValue("thumbnail", thumbnail);
        record.setValue("capturedate", fm.captureDate());
        record.setValue("subject", sid);
        record.setValue("source", fm.source());
        record.setValue("study", fm.studyId());
        table.setRecord( 0, record);
        table.submitAll();
    }   // end if
    else    // Add since this is a new image
    {
        iid = _imageId++;
        QSqlQuery q;
        q.prepare( "INSERT INTO images( id, filepath, imageid, thumbnail, capturedate, subject, source, study) VALUES( ?, ?, ?, ?, ?, ?, ?, ?)");
        q.addBindValue( iid);
        q.addBindValue( absFilePath);
        q.addBindValue( fm.imageId());
        q.addBindValue( thumbnail);
        q.addBindValue( fm.captureDate());
        q.addBindValue( sid);
        q.addBindValue( fm.source());
        q.addBindValue( fm.studyId());
        const bool okay = q.exec();
        assert(okay);
        if ( !okay)
            std::cerr << "[ERR] FaceTools::FileIO::FaceModelDatabase::refreshImage: INSERT image failed!\n";
        newImage = true;
    }   // end else

    return newImage;
}   // end _refreshImage


QPixmap FaceModelDatabase::imageThumbnail( const QString &abspath)
{
    QPixmap pmap;
    QSqlQuery q( QString("SELECT thumbnail FROM images WHERE filepath = '%1'").arg( abspath));
    if ( q.next())
        pmap.loadFromData( q.value(0).toByteArray());
    return pmap;
}   // end imageThumbnail


size_t FaceModelDatabase::numImages( const QString &subjectId)
{
    size_t nimgs = 0;
    QSqlQuery q( QString( "SELECT COUNT(*) FROM images INNER JOIN subjects ON subjects.id = images.subject WHERE subjects.identifier = '%1'").arg(subjectId));
    if ( q.next())
        nimgs = q.value(0).toInt();
    else
        std::cerr << "[ERR] FaceTools::FileIO::FaceModelDatabase::numImages: Image count error!\n";
    return nimgs;
}   // end numImages


size_t FaceModelDatabase::numImages()
{
    QSqlQuery q( "SELECT COUNT(*) FROM images");
    return q.next() ? q.value(0).toInt() : 0;
}   // end numImages


size_t FaceModelDatabase::numSubjects()
{
    QSqlQuery q( "SELECT COUNT(*) FROM subjects");
    return q.next() ? q.value(0).toInt() : 0;
}   // end numSubjects


void FaceModelDatabase::minMaxBirthDates( QDate &min, QDate &max)
{
    QSqlQuery q( "SELECT MIN(birthdate), MAX(birthdate) FROM subjects");
    if ( q.next())
    {
        min = q.value(0).toDate();
        max = q.value(1).toDate();
    }   // end if
}   // end minMaxBirthDates


void FaceModelDatabase::minMaxImageDates( QDate &min, QDate &max)
{
    QSqlQuery q( "SELECT MIN(capturedate), MAX(capturedate) FROM images");
    if ( q.next())
    {
        min = q.value(0).toDate();
        max = q.value(1).toDate();
    }   // end if
}   // end minMaxImageDates


bool FaceModelDatabase::subjectMeta( const QString &subjectId, int8_t &sex, QDate &dob, int &meth, int &peth)
{
    bool found = false;
    QSqlQuery q( QString("SELECT sex, birthdate, maternalethnicity, paternalethnicity FROM subjects WHERE identifier = '%1'").arg(subjectId));
    if ( q.next())
    {
        found = true;
        sex = q.value(0).toInt();
        dob = q.value(1).toDate();
        meth = q.value(2).toInt();
        peth = q.value(3).toInt();
    }   // end else if
    return found;
}   // end subjectMeta


bool FaceModelDatabase::isSubjectMetaMatched( const QString &subjectId, int8_t sex, const QDate &dob, int meth, int peth)
{
    int8_t osex;
    QDate odob;
    int ometh, opeth;
    bool matched = subjectMeta( subjectId, osex, odob, ometh, opeth);
    if ( matched)
        matched = (dob == odob) && (sex == osex) && (meth == ometh) && (peth == opeth);
    return matched;
}   // end isSubjectMetaMatched


bool FaceModelDatabase::init()
{
    if ( _isInit)
        return true;

    static const QString errBase( "[ERR] FaceTools::FileIO::FaceModelDatabase::init: %1\n");
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
    {
        std::cerr << errBase.arg("SQLITE database driver unavailable!").toStdString();
        return false;
    }   // end if

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open())
    {
        std::cerr << errBase.arg("Unable to open database connection!").toStdString();
        return false;
    }   // end if

    QSqlQuery q; // Create the schema
    if ( !q.exec( IMAGES_SQL) || !q.exec( SUBJECTS_SQL))
    {
        std::cerr << errBase.arg(q.lastError().text()).toStdString();
        return false;
    }   // end if

    _isInit = true;
    return _isInit;
}   // end init


namespace {
bool _isValidFile( const QString &fpath)
{
    const QFileInfo finfo( fpath);
    static const QString baseErr("[ERR] FaceTools::FileIO::FaceModelDatabase::refreshImage:");
    const QString pathErr = QString("%1 %2").arg(baseErr, fpath);

    if (!finfo.exists())
    {
        std::cerr << QString( "'%1' doesn't exist!\n").arg(pathErr).toStdString();
        return false;
    }   // end if

    // Ignore non-3DF files.
    if ( !finfo.isFile() || !FMM::isPreferredFileFormat( fpath))
    {
        std::cerr << QString( "'%1' is not a 3DF file!\n").arg(pathErr).toStdString();
        return false;
    }   // end if

    if (!finfo.isReadable())
    {
        std::cerr << QString( "'%1' can't be read!\n").arg(pathErr).toStdString();
        return false;
    }   // end if

    return true;
}   // end _isValidFile


void _changeImagePath( const QString &oldAbsPath, const QString &newAbsPath)
{
    if ( oldAbsPath == newAbsPath)  // No update required
        return;
    QSqlQuery q( QString("UPDATE images SET filepath = '%1' WHERE filepath = '%2'").arg(newAbsPath, oldAbsPath));
    if ( !q.isActive())
        std::cerr << "[WARN] FaceTools::FileIO::FaceModelDatabase::_changeImagePath: Unable to update filepath!\n";
}   // end _changeImagePath

}   // end namespace


bool FaceModelDatabase::refreshImage( FM &fm, QString fpath, const QString &oldpath, bool subjectMetaAuth)
{
    if ( fpath.isEmpty())
        fpath = FMM::filepath( fm);
    fpath = QFileInfo( fpath).absoluteFilePath();
    if ( !_isValidFile(fpath))
        return false;
    // If the old path is given (and is a valid 3DF) first update to new path
    if ( !oldpath.isEmpty() && FMM::isPreferredFileFormat(oldpath))
        _changeImagePath( QFileInfo(oldpath).absoluteFilePath(), fpath);
    return _refreshImage( fpath, fm, subjectMetaAuth);
}   // end refreshImage


void FaceModelDatabase::clear()
{
    QSqlQuery q;
    q.exec("DELETE FROM subjects");
    q.exec("DELETE FROM images");
    _sbjctId = 0;
    _imageId = 0;
}   // end clear


QSqlRelationalTableModel* FaceModelDatabase::createModel() { return _createRelationalModel();}
