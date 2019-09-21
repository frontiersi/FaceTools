/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_ACTION_FACE_MODEL_STATE_H
#define FACE_TOOLS_ACTION_FACE_MODEL_STATE_H

#include <FaceModel.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT FaceModelState
{
public:
    using Ptr = std::shared_ptr<FaceModelState>;
    static Ptr create( FM*, EventGroup);

    inline FM* model() const { return _fm;}

    void restore( const EventGroup&) const;   // Called by UndoState

private:
    FM *_fm;
    bool _metaSaved, _modelSaved;
    RFeatures::ObjModel::Ptr _model;
    std::vector<RFeatures::ObjModelBounds::Ptr> _bnds;
    FaceAssessment::Ptr _ass;
    QString _source;    // Image source
    QString _studyId;   // Study ID info
    QDate _dob;         // Subject date of birth
    int8_t _sex;        // Subject sex
    int _methnicity;    // Subject's maternal ethnicity
    int _pethnicity;    // Subject's paternal ethnicity
    QDate _cdate;       // Date of image capture
    cv::Matx44d _tmat;

    FaceModelState( FM*, EventGroup);
    FaceModelState( const FaceModelState&) = delete;
    FaceModelState& operator=( const FaceModelState&) = delete;
    ~FaceModelState(){}
};  // end class

}}   // end namespaces

#endif
