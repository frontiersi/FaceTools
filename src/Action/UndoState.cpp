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

#include <Action/UndoState.h>
#include <Action/UndoStates.h>
#include <Action/FaceAction.h>
#include <cassert>
using FaceTools::Action::UndoState;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::ModelSelect;


void FaceTools::Action::storeUndo( const FaceAction* a, Event egrp, bool autoRestore)
{
    UndoStates::storeUndo( a, egrp, autoRestore);
}   // end storeUndo


void FaceTools::Action::scrapLastUndo( const FM *fm)
{
    UndoStates::scrapLastUndo( fm);
}   // end scrapLastUndo


UndoState::Ptr UndoState::create( const FaceAction* a, Event egrp, bool autoRestore)
{
    return Ptr( new UndoState(a, egrp, autoRestore), [](UndoState* x){ delete x;});
}   // end create


UndoState::UndoState( const FaceAction* a, Event egrp, bool ar)
    : _action( const_cast<FaceAction*>(a)), _egrp(egrp), _autoRestore(ar),
      _name(a->displayName()), _sfm( MS::selectedModel()) // Could be null
{
    // If auto-restoring, backup the needed data elements otherwise the setUserData function will be used.
    if ( isAutoRestore())
    {
        FMS fms;    // Get the models to save state for

        if ( has( egrp, Event::ALL_VIEWERS))
        {
            for ( FMV* fmv : MS::viewers())
            {
                const FMS fms1 = fmv->attached().models();
                fms.insert( fms1.begin(), fms1.end());
            }   // end for
        }   // end if
        else if ( has( egrp, Event::ALL_VIEWS) && MS::selectedViewer())
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


Event UndoState::restore() const
{
    assert(_action != nullptr);
    if ( !isAutoRestore())
        _action->restoreState(*this);    // Call the custom restore function
    else
        for ( const auto& fstate : _fstates)
            fstate->restore(_egrp);
    return _egrp | Event::RESTORE_CHANGE;
}   // end restore
