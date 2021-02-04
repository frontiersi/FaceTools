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

#include <Interactor/SelectNotifier.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::SelectNotifier;
using FaceTools::Vis::FV;


SelectNotifier::SelectNotifier() : _locked(false), _selected(nullptr) {}


void SelectNotifier::add( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( fv != _selected);
    assert( !_available.has(fv));
    _available.insert(fv);
}   // end add


void SelectNotifier::remove( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( _selected != fv);
    assert( _available.has(fv));
    _available.erase(fv);
}   // end remove


void SelectNotifier::setSelected( FV* fv, bool selected)
{
    if ( !isLocked() && fv && (fv == _selected) != selected)    // Only change if needing to
    {
        if ( _selected)
        {
            _selected->actor()->SetPickable(false);
            emit onSelected( _selected, false);
        }   // end if

        _selected = nullptr;

        if ( selected)
        {
            assert( _available.has(fv));
            _selected = fv;
            _selected->actor()->SetPickable(true);
            fv->viewer()->setSelected(fv);
            emit onSelected( fv, true);
        }   // end if
    }   // end if
}   // end setSelected
