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

#include <Action/UndoState.h>
#include <Action/UndoStates.h>
#include <Action/FaceAction.h>
#include <cassert>
using FaceTools::Action::UndoState;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventGroup;
using MS = FaceTools::Action::ModelSelector;


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
    : _action( const_cast<FaceAction*>(a)), _egrp(egrp), _autoRestore(ar),
      _name(a->displayName()), _sfm( MS::selectedModel()) // Could be null
{
    // If auto-restoring, backup the needed data elements otherwise the setUserData function will be used.
    if ( isAutoRestore())
    {
        FMS fms;    // Get the models to save state for

        if ( egrp.has(Event::ALL_VIEWERS))
        {
            for ( FMV* fmv : MS::viewers())
            {
                const FMS fms1 = fmv->attached().models();
                fms.insert( fms1.begin(), fms1.end());
            }   // end for
        }   // end if
        else if ( egrp.has(Event::ALL_VIEWS) && MS::selectedViewer())
            fms = MS::selectedViewer()->attached().models();
        else if ( _sfm)
            fms.insert(_sfm);

        for ( FM* fm : fms)
            _fstates.push_back( FaceModelState::create(fm, egrp));
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

    if ( !isAutoRestore())
    {
        _sfm->lockForWrite();
        _action->restoreState(this);    // Call the custom restore function
        _sfm->unlock();
    }   // end if
    else
    {
        for ( const auto& fstate : _fstates)
            fstate->restore(_egrp);
    }   // end if

    _action->onEvent(_egrp);
}   // end restore
