/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ModelSelectInteractor.h>
#include <ModelViewer.h>
#include <cassert>
using FaceTools::Interactor::ModelSelectInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;


ModelSelectInteractor::ModelSelectInteractor() : _selected(nullptr)
{
}   // end ctor


void ModelSelectInteractor::add( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( !isSelected(fv));
    assert( !isAvailable(fv));
    _available.insert(fv);
}   // end add


void ModelSelectInteractor::remove( FV* fv)
{
    assert(fv);
    if ( !fv)
        return;
    assert( isAvailable(fv));
    _available.erase(fv);
    if ( isSelected(fv))
        setSelected( fv, false);
}   // end remove


void ModelSelectInteractor::setSelected( FV* fv, bool selected)
{
    if ( !fv)
        return;

    if ( isSelected(fv) == selected)    // Only change if needing to
        return;

    eraseSelected();
    if ( selected)
    {
        assert( _available.has(fv));
        _selected = fv;
        emit onSelected( fv, true);
    }   // end if
}   // end setSelected


// private
void ModelSelectInteractor::eraseSelected()
{
    FV* fv = _selected;
    _selected = nullptr;
    if ( fv)
        emit onSelected( fv, false);
}   // end eraseSelected


FV* ModelSelectInteractor::underPoint( const QPoint& p) const
{
    return _available.find( viewer()->getPointedAt(p));
}   // end underPoint


bool ModelSelectInteractor::leftButtonDown(const QPoint& p)
{
    setSelected( underPoint(p), true);
    return false;
}   // end leftButtonDown


bool ModelSelectInteractor::rightButtonDown( const QPoint& p) { return leftButtonDown(p);}


bool ModelSelectInteractor::leftDoubleClick( const QPoint& p)
{
    FV* fv = underPoint(p);
    if ( fv)
        setSelected( fv, true);
    else // deselect if double clicked off a model
        eraseSelected();
    return true;   // All double clicks swallowed
}   // end leftDoubleClick

