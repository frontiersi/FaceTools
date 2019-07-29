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


void FaceTools::Action::storeUndo( const FaceAction* a, EventGroup egrp)
{
    UndoStates::storeUndo( a, egrp, true);
}   // end storeUndo


void FaceTools::Action::storeUndo( const FaceAction* a)
{
    EventGroup egrp;
    UndoStates::storeUndo( a, egrp, false);
}   // end storeUndo


UndoState::Ptr UndoState::create( const FaceAction* a, EventGroup egrp, bool autoRestore)
{
    return Ptr( new UndoState(a, egrp, autoRestore), [](UndoState* x){ delete x;});
}   // end create


UndoState::UndoState( const FaceAction* a, EventGroup egrp, bool ar)
    : _action( const_cast<FaceAction*>(a)), _egrp(egrp), _autoRestore(ar)
{
    _name = a->displayName();
    _fm = ModelSelector::selectedModel();
    _metaSaved = _fm->isMetaSaved();
    _modelSaved = _fm->isModelSaved();

    // If auto-restoring, backup the needed data elements.
    if ( _autoRestore)
    {
        if ( egrp.has(Event::GEOMETRY_CHANGE) || egrp.has(Event::ORIENTATION_CHANGE))
        {
            _model = _fm->_model->deepCopy(true/*share materials*/);
            _kdtree = _fm->_kdtree;
            _bnds.clear();    // Just in case
            for ( const auto& b : _fm->_bnds)
                _bnds.push_back( b->deepCopy());
        }   // end if

        if ( egrp.has(Event::AFFINE_CHANGE))
            _tmat = _fm->model().transformMatrix();

        if ( egrp.has(Event::CONNECTIVITY_CHANGE))
            _manifolds = _fm->_manifolds;

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
    }   // end if
}   // end ctor


void UndoState::setUserData( const QString& s, const QVariant& v)
{
    _udata[s] = v;
}   // end setUserData


QVariant UndoState::userData( const QString& s) const
{
    assert( _udata.contains(s));
    return _udata[s];
}   // end userData


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
            _fm->_model = _model;
            _fm->_kdtree = _kdtree;
            _fm->_bnds = _bnds;
        }   // end if

        if ( _egrp.has(Event::AFFINE_CHANGE))
            _fm->addTransformMatrix( _tmat * _fm->model().transformMatrix().inv());

        if ( _egrp.has(Event::CONNECTIVITY_CHANGE))
            _fm->_manifolds = _manifolds;

        if ( _egrp.has(Event::GEOMETRY_CHANGE)
          || _egrp.has(Event::LANDMARKS_CHANGE)
          || _egrp.has(Event::ASSESSMENT_CHANGE)
          || _egrp.has(Event::FACE_DETECTED)
          || _egrp.has(Event::PATHS_CHANGE))
            _fm->setAssessment( _ass);

        if ( _egrp.has(Event::METADATA_CHANGE))
        {
            _fm->_source = _source;
            _fm->_studyId = _studyId;
            _fm->_dob = _dob;
            _fm->_sex = _sex;
            _fm->_methnicity = _methnicity;
            _fm->_pethnicity = _pethnicity;
            _fm->_cdate = _cdate;
        }   // end if
    }   // end if

    _fm->setMetaSaved( _metaSaved);
    _fm->setModelSaved( _modelSaved);
    _fm->unlock();

    _action->onEvent(_egrp);
}   // end _restore
