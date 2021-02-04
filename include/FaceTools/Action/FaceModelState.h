/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <FaceTools/FaceModel.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT FaceModelState
{
public:
    using Ptr = std::shared_ptr<FaceModelState>;
    static Ptr create( FM*, Event);

    inline FM* model() const { return _fm;}

    void restore( const Event&) const;   // Called by UndoState

private:
    FM *_fm;
    bool _metaSaved;
    bool _modelSaved;

    QString _source;    // Image source
    QString _studyId;   // Study ID
    QString _subjectId; // Subject ID
    QString _imageId;   // Image ID
    QDate _dob;         // Subject date of birth
    int8_t _sex;        // Subject sex
    int _methnicity;    // Subject's maternal ethnicity
    int _pethnicity;    // Subject's paternal ethnicity
    QDate _cdate;       // Date of image capture

    r3d::Mesh::Ptr _mesh;
    r3d::KDTree::Ptr _kdtree;
    r3d::Manifolds::Ptr _manifolds;

    std::vector<r3d::Bounds::Ptr> _bnds;

    r3d::Mesh::Ptr _mask;
    r3d::KDTree::Ptr _mkdtree;
    size_t _maskHash;

    QMap<int, FaceAssessment::Ptr> _ass;
    FaceAssessment::Ptr _cass;

    Mat4f _tmat;
    std::vector<r3d::CameraParams> _cameras;

    void _saveMesh();
    void _restoreMesh() const;
    void _saveMask();
    void _restoreMask() const;
    void _saveBounds();
    void _restoreBounds() const;
    void _saveMetaData();
    void _restoreMetaData() const;
    void _saveAssessments();
    void _restoreAssessments() const;

    void _saveCameras( const Event&);
    void _restoreCameras( const Event&) const;

    FaceModelState( FM*, Event);
    FaceModelState( const FaceModelState&) = delete;
    FaceModelState& operator=( const FaceModelState&) = delete;
    ~FaceModelState(){}
};  // end class

}}   // end namespaces

#endif
