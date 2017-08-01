#ifndef FACE_TOOLS_FACE_REGISTRATION_MAPPER_H
#define FACE_TOOLS_FACE_REGISTRATION_MAPPER_H

#include "ObjMetaData.h"

namespace FaceTools
{

class FaceTools_EXPORT FaceRegistrationMapper
{
public:
    explicit FaceRegistrationMapper( ObjMetaData::Ptr);

    void generate();

private:
    ObjMetaData::Ptr _omd;
};  // end class

}   // end namespace

#endif
