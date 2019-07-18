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

#include <SelectMouseHandler.h>
#include <FaceModelViewer.h>
#include <ModelSelector.h>
#include <cassert>
using FaceTools::Interactor::SelectMouseHandler;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


SelectMouseHandler::SelectMouseHandler() : _selected(nullptr)
{
}   // end ctor


void SelectMouseHandler::add( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( fv != _selected);
    assert( !_available.has(fv));
    _available.insert(fv);
    setSelected( fv, true);
}   // end add


void SelectMouseHandler::remove( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( _available.has(fv));
    _available.erase(fv);
    setSelected( fv, false);
}   // end remove


void SelectMouseHandler::setSelected( FV* fv, bool selected)
{
    if ( fv && (fv == _selected) != selected)    // Only change if needing to
    {
        eraseSelected();
        if ( selected)
        {
            assert( _available.has(fv));
            _selected = fv;
            emit onSelected( fv, true);
        }   // end if
    }   // end if
}   // end setSelected


// private
void SelectMouseHandler::eraseSelected()
{
    FV* fv = _selected;
    _selected = nullptr;
    if ( fv)
        emit onSelected( fv, false);
}   // end eraseSelected


FV* SelectMouseHandler::underPoint() const
{
    return _available.find( MS::mouseViewer()->getPointedAt(MS::mousePos()));
}   // end underPoint


bool SelectMouseHandler::leftButtonDown()
{
    setSelected( underPoint(), true);
    return false;
}   // end leftButtonDown


bool SelectMouseHandler::rightButtonDown()
{
    setSelected( underPoint(), true);
    return false;
}   // end rightButtonDown


bool SelectMouseHandler::leftDoubleClick()
{
    FV* fv = underPoint();
    if ( fv)
        setSelected( fv, true);
    else // deselect if double clicked off a model
        eraseSelected();
    return false;
}   // end leftDoubleClick
