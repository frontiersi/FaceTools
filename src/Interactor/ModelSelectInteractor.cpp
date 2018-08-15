/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
using FaceTools::FaceControl;


ModelSelectInteractor::ModelSelectInteractor() : _selected(nullptr)
{
}   // end ctor


void ModelSelectInteractor::add( FaceControl* fc)
{
    assert(fc);
    if ( !fc)
        return;
    assert( !isSelected(fc));
    assert( !isAvailable(fc));
    _available.insert(fc);
    setSelected( fc, true);
}   // end add


void ModelSelectInteractor::remove( FaceControl* fc)
{
    assert(fc);
    if ( !fc)
        return;
    assert( isAvailable(fc));
    _available.erase(fc);
    setSelected( fc, false);
}   // end remove


void ModelSelectInteractor::setSelected( FaceControl* fc, bool selected)
{
    if ( !fc)
        return;

    if ( isSelected(fc) == selected)    // Only change if needing to
        return;

    eraseSelected();
    if ( selected)
    {
        assert( _available.has(fc));
        _selected = fc;
        emit onSelected( fc, true);
    }   // end if
}   // end setSelected


// private
void ModelSelectInteractor::eraseSelected()
{
    FaceControl* fc = _selected;
    _selected = nullptr;
    if ( fc)
        emit onSelected( fc, false);
}   // end eraseSelected


FaceControl* ModelSelectInteractor::underPoint( const QPoint& p) const
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
    FaceControl* fc = underPoint(p);
    if ( fc)
        setSelected( fc, true);
    else // deselect if double clicked off a model
        eraseSelected();
    return true;   // All double clicks swallowed
}   // end leftDoubleClick

