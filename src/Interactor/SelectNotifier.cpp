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

#include <Interactor/SelectNotifier.h>
#include <Action/ModelSelector.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::SelectNotifier;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


SelectNotifier::SelectNotifier() : _selected(nullptr)
{
}   // end ctor


void SelectNotifier::add( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( fv != _selected);
    assert( !_available.has(fv));
    _available.insert(fv);
    setSelected( fv, true);
}   // end add


void SelectNotifier::remove( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( _available.has(fv));
    _available.erase(fv);
    setSelected( fv, false);
}   // end remove


void SelectNotifier::setSelected( FV* fv, bool selected)
{
    if ( fv && (fv == _selected) != selected)    // Only change if needing to
    {
        _eraseSelected();
        if ( selected)
        {
            assert( _available.has(fv));
            _selected = fv;
            emit onSelected( fv, true);
        }   // end if
    }   // end if
}   // end setSelected


void SelectNotifier::_eraseSelected()
{
    FV* fv = _selected;
    _selected = nullptr;
    if ( fv)
        emit onSelected( fv, false);
}   // end _eraseSelected


FV* SelectNotifier::_underPoint() const
{
    return _available.find( MS::mouseViewer()->getPointedAt(MS::mousePos()));
}   // end _underPoint


bool SelectNotifier::leftButtonDown()
{
    setSelected( _underPoint(), true);
    return false;
}   // end leftButtonDown


bool SelectNotifier::rightButtonDown()
{
    setSelected( _underPoint(), true);
    return false;
}   // end rightButtonDown


bool SelectNotifier::leftDoubleClick()
{
    FV* fv = _underPoint();
    if ( fv)
        setSelected( fv, true);
    else // deselect if double clicked off a model
        _eraseSelected();
    return false;
}   // end leftDoubleClick
