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
#include <FaceModel.h>
#include <cassert>
using FaceTools::Interactor::SelectNotifier;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FMV;
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
    FMV *fmv = mouseViewer();   // Current viewer the mouse is in
    return _available.find( fmv->getPointedAt( fmv->mouseCoords()));
}   // end _underPoint


FV* SelectNotifier::_findSelected() const
{
    FV *nfv = _underPoint();
    if ( !nfv && _selected && (_selected->viewer() != mouseViewer()))
    {
        // If the mouse viewer has only a single view attached, make this the selected one.
        if ( mouseViewer()->attached().size() == 1)
            nfv = mouseViewer()->attached().first();
        else
        {
            // Otherwise, set the selected view of the currently selected model as the one having fmv as its viewer.
            for ( FV *fv : _selected->data()->fvs())
            {
                if ( fv->viewer() == mouseViewer())
                {
                    nfv = fv;
                    break;
                }   // end if
            }   // end for
        }   // end else
    }   // end if
    return nfv;
}   // end _findSelected


bool SelectNotifier::leftButtonDown()
{
    setSelected( _findSelected(), true);
    return false;
}   // end leftButtonDown


bool SelectNotifier::leftButtonUp()
{
    emit onLeftButtonUp();
    return false;
}   // end leftButtonUp


bool SelectNotifier::leftDoubleClick()
{
    FV* fv = _findSelected();
    if ( fv)
    {
        setSelected( fv, true);
        emit onDoubleClickedSelected();
    }   // end if

    return false;
}   // end leftDoubleClick


bool SelectNotifier::rightButtonDown() { return leftButtonDown();}
bool SelectNotifier::rightDoubleClick() { return leftDoubleClick();}
