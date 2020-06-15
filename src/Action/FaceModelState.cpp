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

#include <Action/FaceModelState.h>
#include <Action/ModelSelector.h>
#include <FaceModel.h>
using FaceTools::Action::FaceModelState;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


FaceModelState::Ptr FaceModelState::create( FM* fm, Event e)
{
    return Ptr( new FaceModelState( fm, e), [](FaceModelState* x){ delete x;});
}   // end create


void FaceModelState::_saveMesh()
{
    _mesh = _fm->_mesh;
    _kdtree = _fm->_kdtree;
    _manifolds = _fm->_manifolds;
}   // end _saveMesh


void FaceModelState::_restoreMesh() const
{
    assert(_mesh);
    _fm->_mesh = _mesh;
    _fm->_kdtree = _kdtree;
    _fm->_manifolds = _manifolds;
}   // end _restoreMesh


void FaceModelState::_saveBounds()
{
    const size_t n = _fm->_bnds.size();
    _bnds.resize(n);
    for ( size_t i = 0; i < n; ++i)
        _bnds[i] = _fm->_bnds[i]->deepCopy();
}   // end _saveBounds


void FaceModelState::_restoreBounds() const
{
    _fm->_bnds = _bnds;
}   // end _restoreBounds


void FaceModelState::_saveMask()
{
    _mask = _fm->_mask;
    _mkdtree = _fm->_mkdtree;
    _maskHash = _fm->_maskHash;
}   // end _saveMask


void FaceModelState::_restoreMask() const
{
    _fm->_mask = _mask;
    _fm->_mkdtree = _mkdtree;
    _fm->_maskHash = _maskHash;
}   // end _restoreMask


void FaceModelState::_saveMetaData()
{
    _source = _fm->_source;
    _studyId = _fm->_studyId;
    _dob = _fm->_dob;
    _sex = _fm->_sex;
    _methnicity = _fm->_methnicity;
    _pethnicity = _fm->_pethnicity;
    _cdate = _fm->_cdate;
}   // end _saveMetaDataa


void FaceModelState::_restoreMetaData() const
{
    _fm->_source = _source;
    _fm->_studyId = _studyId;
    _fm->_dob = _dob;
    _fm->_sex = _sex;
    _fm->_methnicity = _methnicity;
    _fm->_pethnicity = _pethnicity;
    _fm->_cdate = _cdate;
}   // end _restoreMetaData


void FaceModelState::_saveCameras( const Event &egrp)
{
    _cameras.resize(3);
    if ( has( egrp, Event::ALL_VIEWERS))
    {
        _cameras[0] = MS::viewers()[0]->camera();
        _cameras[1] = MS::viewers()[1]->camera();
        _cameras[2] = MS::viewers()[2]->camera();
    }   // end if
    else if ( has( egrp, Event::ALL_VIEWS))
    {
        FMVS fmvs = _fm->fvs().viewers();
        int i = 0;
        for ( const FMV *fmv : fmvs)
            _cameras[i++] = fmv->camera();
    }   // end else if
    else
        _cameras[0]= MS::selectedViewer()->camera();
}   // end _saveCameras


void FaceModelState::_restoreCameras( const Event &egrp) const
{
    if ( has( egrp, Event::ALL_VIEWERS))
    {
        MS::viewers()[0]->setCamera(_cameras[0]);
        MS::viewers()[1]->setCamera(_cameras[1]);
        MS::viewers()[2]->setCamera(_cameras[2]);
    }   // end if
    else if ( has( egrp, Event::ALL_VIEWS))
    {
        FMVS fmvs = _fm->fvs().viewers();
        int i = 0;
        for ( FMV *fmv : fmvs)
            fmv->setCamera(_cameras[i++]);
    }   // end else if
    else
        MS::selectedViewer()->setCamera(_cameras[0]);
}   // end _restoreCameras


void FaceModelState::_saveAssessments()
{
    _ass.clear();
    for ( const auto& a : _fm->_ass)
        _ass[a->id()] = a->deepCopy();
    _cass = _ass[_fm->_cass->id()];
    _saveMetaData();
}   // end _saveAssessments


void FaceModelState::_restoreAssessments() const
{
    _fm->_ass = _ass;
    _fm->_cass = _cass;
    _fm->remakeBounds();
    _restoreMetaData();
}   // end _restoreAssessments


FaceModelState::FaceModelState( FM* fm, Event egrp)
{
    _fm = fm;
    _metaSaved = _fm->isMetaSaved();
    _modelSaved = _fm->isModelSaved();
    _tmat = Mat4f::Identity();

    if ( has( egrp, Event::MESH_CHANGE | Event::CONNECTIVITY_CHANGE))
        _saveMesh();

    if ( has( egrp, Event::MASK_CHANGE | Event::MESH_CHANGE | Event::CONNECTIVITY_CHANGE))
        _saveMask();

    if ( has( egrp, Event::MESH_CHANGE | Event::MASK_CHANGE))
        _saveBounds();

    if ( has( egrp, Event::AFFINE_CHANGE))
        _tmat = _fm->mesh().transformMatrix();

    if ( has( egrp, Event::MESH_CHANGE | Event::LANDMARKS_CHANGE | Event::CONNECTIVITY_CHANGE | Event::ASSESSMENT_CHANGE | Event::PATHS_CHANGE))
        _saveAssessments();

    if ( has( egrp, Event::CAMERA_CHANGE))
        _saveCameras( egrp);
}   // end ctor


void FaceModelState::restore( const Event &egrp) const
{
    _fm->lockForWrite();

    if ( has( egrp, Event::MESH_CHANGE | Event::CONNECTIVITY_CHANGE))
        _restoreMesh();

    if ( has( egrp, Event::MASK_CHANGE | Event::MESH_CHANGE | Event::CONNECTIVITY_CHANGE))
        _restoreMask();

    if ( has( egrp, Event::MESH_CHANGE | Event::MASK_CHANGE))
        _restoreBounds();

    if ( has( egrp, Event::AFFINE_CHANGE))
        _fm->addTransformMatrix( _tmat * _fm->mesh().inverseTransformMatrix());
    
    if ( has( egrp, Event::MESH_CHANGE | Event::LANDMARKS_CHANGE | Event::CONNECTIVITY_CHANGE | Event::ASSESSMENT_CHANGE | Event::PATHS_CHANGE))
        _restoreAssessments();

    if ( has( egrp, Event::CAMERA_CHANGE))
        _restoreCameras( egrp);

    _fm->setMetaSaved( _metaSaved);
    _fm->setModelSaved( _modelSaved);
    _fm->unlock();
}   // end restore
