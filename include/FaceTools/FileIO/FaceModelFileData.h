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

#ifndef FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_DATA_H
#define FACE_TOOLS_FILE_IO_FACE_MODEL_FILE_DATA_H

#include "FaceModelXMLFileHandler.h"
#include <FaceTools/Ethnicities.h>

namespace FaceTools { namespace FileIO {

enum struct Content : uint8_t
{
    EMPTY        = 0x00,    // Nothing
    SEX          = 0x01,    // Sex string
    AGE          = 0x02,    // Date of birth, capture date, and age
    ETHNICITY    = 0x04,    // Maternal and paternal ethnicity strings
    IMAGE        = 0x08,    // Study identifier, source, and current assessor's notes
    LANDMARKS    = 0x10,
    MEASUREMENTS = 0x20,
    PATHS        = 0x40,
    HPOS         = 0x80
};  // end enum

// Create and return a new Content as a union of two others.
FaceTools_EXPORT Content operator|( const Content&, const Content&);

// Create and return a new Content as an intersection of two others.
FaceTools_EXPORT Content operator&( const Content&, const Content&);


class FaceTools_EXPORT FaceModelFileData
{
public:
    /**
     * Pass in a path to a 3DF file to read in the assessment for the given assessor,
     * or the default assessment if left empty. If the named assessor is not found,
     * the default assessment is used. Check successful read with fn error().
     */
    explicit FaceModelFileData( const QString &fpath, const QString &assessorName="");
    explicit FaceModelFileData( const FM&);
    ~FaceModelFileData();

    inline const QString &error() const { return _err;}

    inline const QDate& dateOfBirth() const { return _fm->dateOfBirth();}
    inline const QDate& captureDate() const { return _fm->captureDate();}
    inline float age() const { return _fm->age();}

    inline QString sex() const { return toSexString(_fm->sex());}
    inline int maternalEthnicity() const { return _fm->maternalEthnicity();}
    inline int paternalEthnicity() const { return _fm->paternalEthnicity();}
    inline const QString& maternalEthnicityString() const { return Ethnicities::name( maternalEthnicity());}
    inline const QString& paternalEthnicityString() const { return Ethnicities::name( paternalEthnicity());}

    inline const QString& source() const { return _fm->source();}
    inline const QString& studyId() const { return _fm->studyId();}
    inline const QString& notes() const { return _fm->currentAssessment()->notes();}
    inline const QString& assessor() const { return _fm->currentAssessment()->assessor();}

    inline const Landmark::LandmarkSet& landmarks() const { return _fm->currentLandmarks();}
    inline const PathSet& paths() const { return _fm->currentPaths();}

    /**
     * Return the dimension value of the given measurement on the given lateral.
     * Defaults to the first (and possibly only) dimension for the metric.
     */
    float measurementValue( int mid, FaceLateral, size_t dim=0) const;

    /**
     * Output data in CSV format. Pass in a binary union of the data to output to the given stream.
     * If left default (empty) all content are exported.
     */
    void toCSV( std::ostream&, Content=Content::EMPTY) const;

private:
    const FM *_fm;
    FM _ifm;
    QString _err;
    struct HPO;
    std::vector<HPO*> _hpos;

    FaceAssessment& _setCurrentAssessment( const QString&);

    void _printSummaryLine( std::ostream&, Content) const;
    void _printSectionLine( std::ostream&, Content) const;
    void _landmarksToCSV( std::ostream&) const;
    void _measurementsToCSV( std::ostream&) const;
    void _pathsToCSV( std::ostream&) const;
    void _hposToCSV( std::ostream&) const;

    void _printMeasurement( int, FaceLateral, const std::string&, size_t, std::ostream&) const;
    void _printLandmark( int, FaceLateral, const std::string&, std::ostream&) const;
    void _readHPOTerms( const PTree&);
    void _clearHPOTerms();

    FaceModelFileData( const FaceModelFileData&) = delete;
    void operator=( const FaceModelFileData&) = delete;
};  // end class

}}   // end namespaces

#endif
