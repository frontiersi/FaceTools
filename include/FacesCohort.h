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
