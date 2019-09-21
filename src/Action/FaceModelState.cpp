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

#include <Action/FaceModelState.h>
#include <FaceModel.h>
using FaceTools::Action::FaceModelState;
using FaceTools::Action::EventGroup;
using FaceTools::FM;


FaceModelState::Ptr FaceModelState::create( FM* fm, EventGroup egrp)
{
    return Ptr( new FaceModelState( fm, egrp), [](FaceModelState* x){ delete x;});
}   // end create


FaceModelState::FaceModelState( FM* fm, EventGroup egrp)
{
    _fm = fm;
    _metaSaved = _fm->isMetaSaved();
    _modelSaved = _fm->isModelSaved();

    if ( egrp.has(Event::GEOMETRY_CHANGE) || egrp.has(Event::ORIENTATION_CHANGE) || egrp.has(Event::CONNECTIVITY_CHANGE))
    {
        _model = _fm->_model->deepCopy(true/*share materials*/);
        _bnds.clear();
        for ( const auto& b : _fm->_bnds)
            _bnds.push_back( b->deepCopy());
    }   // end if

    if ( egrp.has(Event::AFFINE_CHANGE))
        _tmat = _fm->model().transformMatrix();

    if ( egrp.has(Event::GEOMETRY_CHANGE)
      || egrp.has(Event::LANDMARKS_CHANGE)
      || egrp.has(Event::ASSESSMENT_CHANGE)
      || egrp.has(Event::FACE_DETECTED)
      || egrp.has(Event::PATHS_CHANGE))
        _ass = _fm->currentAssessment()->deepCopy();

    if ( egrp.has(Event::METADATA_CHANGE))
    {
        _source = _fm->_source;
        _studyId = _fm->_studyId;
        _dob = _fm->_dob;
        _sex = _fm->_sex;
        _methnicity = _fm->_methnicity;
        _pethnicity = _fm->_pethnicity;
        _cdate = _fm->_cdate;
    }   // end if
}   // end ctor


void FaceModelState::restore( const EventGroup& egrp) const
{
    _fm->lockForWrite();

    if ( _model)
    {
        _fm->update( _model, egrp.has(Event::CONNECTIVITY_CHANGE)); // Also causes orientation to be updated based on model transform or landmarks
        _fm->_bnds = _bnds;
    }   // end if

    if ( egrp.has(Event::AFFINE_CHANGE))
        _fm->addTransformMatrix( _tmat * _fm->model().transformMatrix().inv());
    
    if ( _ass)
        _fm->setAssessment( _ass);

    if ( egrp.has(Event::METADATA_CHANGE))
    {
        _fm->_source = _source;
        _fm->_studyId = _studyId;
        _fm->_dob = _dob;
        _fm->_sex = _sex;
        _fm->_methnicity = _methnicity;
        _fm->_pethnicity = _pethnicity;
        _fm->_cdate = _cdate;
    }   // end if

    _fm->setMetaSaved( _metaSaved);
    _fm->setModelSaved( _modelSaved);
    _fm->unlock();
}   // end restore
