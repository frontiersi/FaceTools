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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_DATABASE_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_DATABASE_H

#include "FaceTools/FaceModel.h"
#include <QPixmap>
#include <QRegExp>
#include <QtSql>
#include <QDir>

/**
 * NB: All paths provided must be absolute!
 */

namespace FaceTools { namespace FileIO {

class FaceTools_EXPORT FaceModelDatabase
{
public:
    // Initialise the database by creating the initial connection and defining the schema.
    static bool init();

    // Clear all tables.
    static void clear();

    // Return the thumbnail of the given image in the DB or an empty pixmap if nothing found.
    static QPixmap imageThumbnail( const QString &imagePath);

    // Update the database from the given model, ensuring the filepath in the images
    // table is updated to be fpath instead of oldpath (if given) otherwise the existing
    // path is assumed (the image is already in the DB in that case). The paths must be 3DF files.
    // If the image is not in the database, it is added. If the subject ID is empty or contains
    // the placeholder text, the subject ID is replaced (regardless of whether subjectMetaAuth
    // is true or not).
    // If subjectMetaAuth is true then the model's subject data is treated as authoritative.
    // If false, the subject meta data is updated on the model from the database upon return
    // and the meta saved flag is set false.
    // Returns true iff the image remains in the database upon return.
    static bool refreshImage( FM&, QString fpath="", const QString &oldpath="", bool subjectMetaAuth=false);

    // Returns the number of images in the database of the same subject.
    static size_t numImages( const QString &subjectId);

    // Returns true iff the given metadata matches the given subject ID and the subject ID was found.
    // Prints error message if multiple rows with the same subject ID found.
    static bool isSubjectMetaMatched( const QString &subjectId, int8_t sex, const QDate &dob, int meth, int peth);

    // Returns the subject's information for the given ID returning true iff the subject was found.
    // If the subject was not found, all of the out parameters will be unchanged.
    static bool subjectMeta( const QString &subjectId, int8_t &sex, QDate &dob, int &meth, int &peth); 

    static QSqlRelationalTableModel* createModel();

    // Constant subject identifier prefix when no subject identifier present.
    static const QString NO_SUBJECT_STRING;
    static const QRegExp NO_SUBJECT_REGEXP;

private:
    static bool _isInit;
    static int _imageId;
    static int _sbjctId;
    static void _refreshImage( const QString&, FM&, bool);
};  // end class

}}   // end namespaces

#endif
