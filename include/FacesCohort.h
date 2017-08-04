/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#ifndef FACE_TOOLS_FACES_COHORT_H
#define FACE_TOOLS_FACES_COHORT_H

#include "FaceTools.h"
#include "ObjMetaData.h"

namespace FaceTools
{

class FaceTools_EXPORT FacesCohort
{
public:
    explicit FacesCohort( const std::string version="1.0");
    virtual ~FacesCohort();

    const std::string& version() const { return _version;}
    void setVersion( const std::string& v) { _version = v;}

    const std::string& description() const { return _desc;}
    void setDescription( const std::string& d) { _desc = d;}

    void addRecord( ObjMetaData::Ptr);
    ObjMetaData::Ptr getRecord( int i) const;

    size_t size() const { return _records.size();}
    const std::vector<ObjMetaData::Ptr>& getRecords() const { return _records;}

    class VersionException : public std::runtime_error
    { public:
        explicit VersionException( const std::string& what) : std::runtime_error(what) {}
    };  // end class

private:
    std::string _version;
    std::string _desc;
    std::vector<ObjMetaData::Ptr> _records;
};  // end class

}   // end namespace


// XML input and output streams
FaceTools_EXPORT std::istream& operator>>( std::istream&, FaceTools::FacesCohort&);  // Catch VersionException
FaceTools_EXPORT std::ostream& operator<<( std::ostream&, const FaceTools::FacesCohort&);

#endif
