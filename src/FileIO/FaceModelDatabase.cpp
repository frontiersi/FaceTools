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
QSqlRecord _imageRecordForFilePath( const QString &abspath)
{
    QSqlRecord rec;
    assert( !abspath.isEmpty());
    QSqlTableModel table;
    table.setTable("images");
    table.setFilter( QString("filepath = '%1'").arg( abspath));
    table.select();
    if ( table.rowCount() > 0)
        rec = table.record(0);
    return rec;
}   // end _imageRecordForFilePath


// Find the image ID (pkey) from the absolute path.
int _getImageKey( const QString &abspath)
{
    const QSqlRecord rec = _imageRecordForFilePath( abspath);
    return rec.isEmpty() ? -1 : rec.value("id").toInt();
}   // end _getImageKey


// Return the current subject key associated with the image.
int _getSubjectKeyFromImagePath( const QString &abspath)
{
    const QSqlRecord rec = _imageRecordForFilePath( abspath);
    return rec.isEmpty() ? -1 : rec.value("subject").toInt();
}   // end _getSubjectKeyFromImagePath


// Returns the primary key from the subjects table for the record
// with the given string identifier or -1 if no record was found.
int _getSubjectKey( const QString &subjectId)
{
    int sid = -1;
    if ( !subjectId.isEmpty())
    {
        QSqlTableModel table;
        table.setTable("subjects");
        table.setFilter( QString("identifier = '%1'").arg( subjectId));
        table.select();
        if ( table.rowCount() > 0)
        {
            if ( table.rowCount() > 1)
                std::cerr << "[WARN] _getSubjectKey returned multiple rows! Must consolidate" << std::endl;
            sid = table.record(0).value("id").toInt();
        }   // end if
    }   // end if
    return sid;
}   // end _getSubjectKey


QString _getSubjectIdentifierFromKey( int sid)
{
    QSqlTableModel table;
    table.setTable("subjects");
    table.setFilter( QString("id = %1").arg( sid));
    table.select();
    return table.rowCount() > 0 ? table.record(0).value("identifier").toString() : "";
}   // end _getSubjectIdentifierFromKey


void _changeImagePath( const QString &oldAbsPath, const QString &newAbsPath)
{
    if ( oldAbsPath == newAbsPath)  // No update required
        return;
    QSqlRecord record = _imageRecordForFilePath( oldAbsPath);
    if ( !record.isEmpty())
    {
        QSqlTableModel table;
        table.setTable("images");
        table.setEditStrategy( QSqlTableModel::OnManualSubmit);
        record.setValue("filepath", newAbsPath);
        table.setRecord( 0, record);
        table.submitAll();
    }   // end if
    else
        std::cerr << "[WARN] FaceTools::FileIO::FaceModelDatabase::_changeImagePath: Old filepath not found!\n";
}   // end _changeImagePath


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
    QSqlTableModel table;
    table.setEditStrategy( QSqlTableModel::OnManualSubmit);
    table.setTable("subjects");
    table.setFilter( QString("id = %1").arg(sid));
    table.select();
    if ( table.rowCount() == 0)
        std::cerr << "[ERR] FaceTools::FileIO::FaceModelDatabase::_updateSubject: Subject not found!\n";
    assert( table.rowCount() == 1);

    QSqlRecord record = table.record(0);

    if ( subjectMetaAuth)    // Update the DB record using data from this model?
    {
        record.setValue("birthdate", fm.dateOfBirth());
        record.setValue("maternalethnicity", fm.maternalEthnicity());
        record.setValue("paternalethnicity", fm.paternalEthnicity());
        record.setValue("sex", fm.sex());
        table.setRecord( 0, record);
        table.submitAll();
    }   // end if
    else    // Update the model using existing record data
    {
        const QDate oldDOB = fm.dateOfBirth();
        const int oldmat = fm.maternalEthnicity();
        const int oldpat = fm.paternalEthnicity();
        const int8_t oldsex = fm.sex();

        const QDate dob = record.value("birthdate").toDate();
        const int meth = record.value("maternalethnicity").toInt();
        const int peth = record.value("paternalethnicity").toInt();
        const int8_t sex = record.value("sex").toInt();

        fm.setDateOfBirth( dob);
        fm.setMaternalEthnicity( meth);
        fm.setPaternalEthnicity( peth);
        fm.setSex( sex);
        fm.setMetaSaved( oldDOB == dob
                      && oldmat == meth
                      && oldpat == peth
                      && oldsex == sex);
    }   // end else
}   // end _updateSubject


size_t _numImagesWithSubjectKey( int sid)
{
    size_t n = 0;
    if ( sid >= 0)
    {
        QSqlTableModel table;
        table.setTable("images");
        table.setFilter( QString("subject = %1").arg( sid));
        table.select();
        n = table.rowCount();
    }   // end if
    return n;
}   // end _numImagesWithSubjectKey


bool _removeSubject( int sid) { return QSqlQuery().exec(QString("DELETE FROM subjects WHERE id = %1").arg(sid));}
bool _removeImage( int iid) { return QSqlQuery().exec(QString("DELETE FROM images WHERE id = %1").arg(iid));}

}   // end namespace


void FaceModelDatabase::_refreshImage( const QString &absFilePath, FM &fm, bool subjectMetaAuth)
{
    int iid = _getImageKey( absFilePath);       // Image already in DB according to file path?
    const bool isUnknownSubject = NO_SUBJECT_REGEXP.exactMatch( fm.subjectId());
    int sid = _getSubjectKey( fm.subjectId());  // Will be -1 if subjectId is empty or subject not yet in DB
    if ( isUnknownSubject)
    {
        // If the subject matched unknown and the image is not already in the database then the subject
        // identifier is set empty. This accounts for the possibility that a 3DF contains the unknown string
        // (shouldn't happen because subject identifiers matching unknown string are removed upon save, but
        // it's possible to edit the 3DF metadata externally so need to check here too).
        if ( iid < 0)
        {
            fm.setSubjectId("");
            sid = -1;
        }   // end if
        else if ( sid >= 0) // Existing subject can't be different from the original for this image if unknown subject!
        {
            const int csid = _getSubjectKeyFromImagePath( absFilePath); // The existing subject
            if ( sid != csid)
            {
                fm.setSubjectId( _getSubjectIdentifierFromKey( csid));  // Reset with old identifier
                sid = csid;
            }   // end if
        }   // end else if
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
    }   // end if
    else if ( !isUnknownSubject) // Update subject in DB (subjectMetaAuth=true), or in model from DB (subjectMetaAuth=false) if valid subject identifier
        _updateSubject( sid, fm, subjectMetaAuth);

    // Write the thumbnail as a byte array for binary large object (BLOB)
    QByteArray thumbnail;
    QBuffer inBuffer( &thumbnail);
    inBuffer.open( QIODevice::WriteOnly);
    fm.thumbnail().save( &inBuffer, "PNG");

    assert( sid >= 0);
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
    }   // end else
}   // end _refreshImage


QPixmap FaceModelDatabase::imageThumbnail( const QString &abspath)
{
    QPixmap pmap;
    const QSqlRecord record = _imageRecordForFilePath( abspath);
    if ( !record.isEmpty())
        pmap.loadFromData( record.value("thumbnail").toByteArray());
    return pmap;
}   // end imageThumbnail


size_t FaceModelDatabase::numImages( const QString &subjectId)
{
    size_t nimgs = 0;
    const int sid = _getSubjectKey( subjectId);
    if ( sid >= 0)
        nimgs = _numImagesWithSubjectKey( sid);
    return nimgs;
}   // end numImages


bool FaceModelDatabase::subjectMeta( const QString &subjectId, int8_t &sex, QDate &dob, int &meth, int &peth)
{
    bool found = false;
    QSqlTableModel table;
    table.setTable("subjects");
    table.setFilter( QString("identifier = '%1'").arg(subjectId));
    table.select();
    if ( table.rowCount() > 1)
        std::cerr << "[ERR] FaceTools::FileIO::FaceModelDatabase::subjectMeta: Subject identifier not unique!\n";
    else if ( table.rowCount() == 1)
    {
        found = true;
        const QSqlRecord record = table.record(0);
        sex = record.value("sex").toInt();
        dob = record.value("birthdate").toDate();
        meth = record.value("maternalethnicity").toInt();
        peth = record.value("paternalethnicity").toInt();
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
}   // end namespace


bool FaceModelDatabase::refreshImage( FM &fm, QString fpath, const QString &oldpath, bool subjectMetaAuth)
{
    if ( fpath.isEmpty())
        fpath = FMM::filepath( fm);
    fpath = QFileInfo( fpath).absoluteFilePath();
    if ( !_isValidFile(fpath))
        return false;

    if ( !oldpath.isEmpty()) // If the old path is given, first update to new path
    {
        if ( !_isValidFile(oldpath))
            return false;
        _changeImagePath( QFileInfo(oldpath).absoluteFilePath(), fpath);
    }   // end if

    _refreshImage( fpath, fm, subjectMetaAuth);
    return true;
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
