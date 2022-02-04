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
#include <FileIO/FaceModelXMLFileHandler.h>
#include <FileIO/FaceModelManager.h>
#include <QMutexLocker>
#include <QtSql>
using FaceTools::FileIO::FaceModelDatabase;

// Static definitions
QMutex FaceTools::FileIO::FaceModelDatabase::_lock;
bool FaceTools::FileIO::FaceModelDatabase::_isInit(false);


namespace {

const auto IMAGES_SQL = QLatin1String(R"(
    CREATE TABLE images( id INT PRIMARY KEY,
                           imageid VARCHAR,
                              path VARCHAR,
                          capturedate DATE,
                      refreshtime DATETIME,
                               subject INT,
                             study VARCHAR,
                            source VARCHAR))");

const auto INSERT_IMAGE_SQL = QLatin1String(R"(
    INSERT INTO images( imageid, path, capturedate, subject, study, source)
               VALUES( ?, ?, ?, ?, ?, ?)
    )");

bool _addImageQuery( QSqlQuery &q, const QString &imageid, const QString &path, const QDate &cdate,
                const QVariant &subjectId, const QString &study, const QString &source)
{
    q.addBindValue( imageid);
    q.addBindValue( path);
    q.addBindValue( cdate);
    q.addBindValue( subjectId);
    q.addBindValue( study);
    q.addBindValue( source);
    return q.exec();
}   // end _addImage

const auto SUBJECTS_SQL = QLatin1String(R"(
    CREATE TABLE subjects( id INT PRIMARY KEY,
                           identifier VARCHAR,
                               birthdate DATE,
                                      sex INT,
                        maternalethnicity INT,
                        paternalethnicity INT))");

const auto INSERT_SUBJECT_SQL = QLatin1String(R"(
    INSERT INTO subjects( identifier, birthdate, sex, maternalethnicity, paternalethnicity)
               VALUES( ?, ?, ?, ?, ?)
    )");

QVariant _addSubjectQuery( QSqlQuery &q, const QString &sid, const QDate &dob, int8_t sex, int meth, int peth)
{
    q.addBindValue( sid);
    q.addBindValue( dob);
    q.addBindValue( sex);
    q.addBindValue( meth);
    q.addBindValue( peth);
    q.exec();
    return q.lastInsertId();
}   // end _addSubject


bool _execSQL( QSqlQuery &q, const QString &cmd, const QString &errBase)
{
    if ( !q.exec( cmd))
    {
        std::cerr << errBase.arg(q.lastError().text()).toStdString();
        return false;
    }   // end if
    return true;
}   // end execSQL

}   // end namespace


bool FaceModelDatabase::init()
{
    if ( _isInit)
        return true;

    static const QString errBase( "[ERR] FaceTools::FileIO::FaceModelDatabase::init: %1\n");
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
    {
        std::cerr << errBase.arg("QSQLITE database driver unavailable!").toStdString();
        return false;
    }   // end if

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open())
    {
        std::cerr << errBase.arg("Unable to open database connection!").toStdString();
        return false;
    }   // end if

    // Create the schema
    QSqlQuery q;
    if ( !_execSQL( q, IMAGES_SQL, errBase))
        return false;
    if ( !_execSQL( q, SUBJECTS_SQL, errBase))
        return false;

    /*
    QSqlRelationalTableModel images;
    images.setTable("images");
    images.setEditStrategy( QSqlTableModel::OnManualSubmit);
    images.setRelation( 5, QSqlRelation("subjects", "id", "identifier"));
    */

    _isInit = true;
    return _isInit;
}   // end init


void FaceModelDatabase::reset()
{
    QMutexLocker lock(&_lock);
    QSqlQuery q;
    q.exec("DELETE FROM subjects");
    q.exec("DELETE FROM images");
}   // end reset


bool FaceModelDatabase::_addImage( const QFileInfo &path)
{
    // Read in metadata only
    QTemporaryDir tdir;
    PTree ptree;
    const QString err = readMeta( path.filePath(), tdir, ptree);
    double fversion = 0.0;
    FM fm;
    if ( !err.isEmpty() || !importMetaData( fm, ptree, fversion))
        return false;

    QSqlTableModel subjects;
    subjects.setTable("subjects");
    subjects.setEditStrategy( QSqlTableModel::OnManualSubmit);
    subjects.setFilter( QString("identifier == %1").arg(fm.subjectId()));
    if ( !subjects.select())
        return false;

    // Get the ID of the subject (primary) from the subjects table or add if not already present.
    QSqlQuery q;
    QVariant subjectId;
    if (subjects.rowCount() == 0)  // Insert the subject if not already found
    {
        if ( !q.prepare( INSERT_SUBJECT_SQL))
            return false;
        subjectId = _addSubjectQuery( q, fm.subjectId(), fm.dateOfBirth(), fm.sex(), fm.maternalEthnicity(), fm.paternalEthnicity());
    }   // end if
    else
        subjectId = subjects.record(0).value("id");

    q.prepare( INSERT_IMAGE_SQL);
    return _addImageQuery( q, fm.imageId(), path.absoluteFilePath(), fm.captureDate(), subjectId, fm.studyId(), fm.source());
}   // end _addImage


int FaceModelDatabase::refresh( const QString &fpath)
{
    static const QString baseErr("[ERR] FaceTools::FileIO::FaceModelDatabase::refresh:");
    const QString pathErr = QString("%1 %2").arg(baseErr, fpath);
    const QString EXT_3DF = FaceModelManager::fileFormats().preferredExt().toLower();

    int rv = -1;
    const QFileInfo path( fpath);
    if ( !path.exists())
        std::cerr << QString( "%1 doesn't exist!\n").arg(pathErr).toStdString();
    else if (!path.isReadable())
        std::cerr << QString( "%1 can't be read!\n").arg(pathErr).toStdString();
    else
    {
        if (path.isFile())
        {
            if ( _addImage( path))
                rv = 1;
            else
            {
                std::cerr << QString( "%1 wasn't added!\n").arg(pathErr).toStdString();
                rv = -1;
            }   // end else
        }   // end if
        else if (path.isDir())
        {
            rv = 0;
            const QDir dir( path.filePath());
            const QFileInfoList flist = dir.entryInfoList({"*." + EXT_3DF}, QDir::Files | QDir::Readable);
            for ( const QFileInfo &pth : flist)
            {
                if ( _addImage( pth))
                    rv++;
                else
                {
                    std::cerr << QString( "%1 %2 wasn't added!\n").arg(baseErr, pth.filePath()).toStdString();
                    rv = -1;
                    break;
                }   // end else
            }   // end for
        }   // end else if
        else
            std::cerr << QString( "%1 is unknown!\n").arg(pathErr).toStdString();
    }   // end else
    return rv;
}   // end refresh
