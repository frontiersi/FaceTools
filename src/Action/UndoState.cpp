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

#include <UndoState.h>
#include <UndoStates.h>
#include <FaceAction.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Action::UndoState;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventGroup;
using FaceTools::Landmark::LandmarkSet;


void FaceTools::Action::storeUndo( const FaceAction* a, EventGroup egrp, bool res)
{
    UndoStates::storeUndo( a, egrp, res);
}   // end storeUndo


UndoState::Ptr UndoState::create( EventGroup egrp, bool autoRestore)
{
    Ptr ustate( new UndoState);
    ustate->_action = nullptr;
    ustate->_egrp = egrp;
    ustate->_autoRestore = autoRestore;
    const FM* fm = ustate->_fm = ModelSelector::selectedModel();

    FM& bfm = ustate->_backm;
    bfm.setMetaSaved( fm->isMetaSaved());
    bfm.setModelSaved( fm->isModelSaved());

    // If auto-restoring, set the data elements that are required for auto-restoration.
    if ( autoRestore)
    {
        if ( egrp.has(Event::GEOMETRY_CHANGE) || egrp.has(Event::ORIENTATION_CHANGE))
        {
            bfm._model = fm->_model->deepCopy(true/*share materials*/);
            bfm._kdtree = fm->_kdtree;
            bfm._bnds.clear();    // Just in case
            for ( const auto& b : fm->_bnds)
                bfm._bnds.push_back( b->deepCopy());
        }   // end if

        if ( egrp.has(Event::AFFINE_CHANGE))
            ustate->_tmat = fm->model().transformMatrix();

        if ( egrp.has(Event::CONNECTIVITY_CHANGE))
            bfm._manifolds = fm->_manifolds;

        if ( egrp.has(Event::GEOMETRY_CHANGE) || egrp.has(Event::LANDMARKS_CHANGE) || egrp.has(Event::FACE_DETECTED))
            bfm._landmarks = fm->_landmarks->deepCopy();

        if ( egrp.has(Event::GEOMETRY_CHANGE) || egrp.has(Event::PATHS_CHANGE))
            bfm._paths = fm->_paths->deepCopy();

        if ( egrp.has(Event::METADATA_CHANGE))
        {
            bfm._notes = fm->_notes;
            bfm._source = fm->_source;
            bfm._studyId = fm->_studyId;
            bfm._dob = fm->_dob;
            bfm._sex = fm->_sex;
            bfm._methnicity = fm->_methnicity;
            bfm._pethnicity = fm->_pethnicity;
            bfm._cdate = fm->_cdate;
        }   // end if
    }   // end if

    return ustate;
}   // end create


void UndoState::restore() const
{
    assert(_action != nullptr);

    _fm->lockForWrite();

    if ( !_autoRestore)
        _action->restoreState(this);    // Call the custom restore function
    else
    {
        if ( _egrp.has(Event::GEOMETRY_CHANGE) || _egrp.has(Event::ORIENTATION_CHANGE))
        {
            _fm->_model = _backm._model;
            _fm->_kdtree = _backm._kdtree;
            _fm->_bnds = _backm._bnds;
        }   // end if

        if ( _egrp.has(Event::AFFINE_CHANGE))
            _fm->addTransformMatrix( _tmat * _fm->model().transformMatrix().inv());

        if ( _egrp.has(Event::CONNECTIVITY_CHANGE))
            _fm->_manifolds = _backm._manifolds;

        if ( _backm._landmarks)
            _fm->_landmarks = _backm._landmarks;

        if ( _backm._paths)
            _fm->_paths = _backm._paths;

        if ( _egrp.has(Event::METADATA_CHANGE))
        {
            _fm->_notes = _backm._notes;
            _fm->_source = _backm._source;
            _fm->_studyId = _backm._studyId;
            _fm->_dob = _backm._dob;
            _fm->_sex = _backm._sex;
            _fm->_methnicity = _backm._methnicity;
            _fm->_pethnicity = _backm._pethnicity;
            _fm->_cdate = _backm._cdate;
        }   // end if
    }   // end if

    _fm->setMetaSaved( _backm.isMetaSaved());
    _fm->setModelSaved( _backm.isModelSaved());
    _fm->unlock();

    _action->onEvent(_egrp);
}   // end restore
