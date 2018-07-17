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


// public
ModelSelectInteractor::ModelSelectInteractor( bool ex)
    : _exclusive(ex), _enabled(true)
{}   // end ctor


// public
void ModelSelectInteractor::enableUserSelect( bool v) { _enabled = v;}
void ModelSelectInteractor::setExclusiveSelect( bool v) { _exclusive = v;}
bool ModelSelectInteractor::isExclusiveSelect() const { return _exclusive;}


// public
void ModelSelectInteractor::add( FaceControl* fc)
{
    assert( !isSelected(fc));
    assert( !isAvailable(fc));
    _available.insert(fc);
    setSelected( fc, true);
}   // end add


// public
void ModelSelectInteractor::remove( FaceControl* fc)
{
    assert( isAvailable(fc));
    _available.erase(fc);
    setSelected( fc, false);
}   // end remove


// public
void ModelSelectInteractor::setSelected( FaceControl* fc, bool selected)
{
    if ( isSelected(fc) == selected)    // Only change if needing to
        return;

    if ( _exclusive)
        deselectAll();
    else
        eraseSelected(fc);

    if ( selected)
    {
        assert( _available.has(fc));
        insertSelected(fc);
    }   // end if
}   // end setSelected


// public
bool ModelSelectInteractor::isSelected( FaceControl* fc) const { return _selected.has(fc);}
bool ModelSelectInteractor::isAvailable( FaceControl* fc) const { return _available.has(fc);}


// private
void ModelSelectInteractor::eraseSelected( FaceControl* fc)
{
    _selected.erase(fc);
    emit onSelected( fc, false);
}   // end eraseSelected


// private
void ModelSelectInteractor::insertSelected( FaceControl* fc)
{
    _selected.insert(fc);
    emit onSelected( fc, true);
}   // end insertSelected


bool ModelSelectInteractor::leftButtonDown(const QPoint& p)
{
    return rightButtonDown(p);
}   // end leftButtonDown


bool ModelSelectInteractor::rightButtonDown( const QPoint& p)
{
    if ( _enabled)
    {
        FaceControl* fc = _available.find( viewer()->getPointedAt(p));
        if ( fc && !isSelected(fc))
        {
            if ( _exclusive)    // signal to listeners that currently selected models are being deselected
                deselectAll();
            insertSelected( fc);
        }   // end if
    }   // end if
    return false;
}   // end rightButtonDown


bool ModelSelectInteractor::leftDoubleClick( const QPoint& p)
{
    bool swallowed = false;
    if ( _enabled)
    {
        swallowed = true;   // All double clicks swallowed
        FaceControl* fc = _available.find( viewer()->getPointedAt(p));
        if ( fc)
        {
            if ( _exclusive)
                deselectAll();

            if ( !isSelected(fc))   // Select after double-clicking on a model
                insertSelected( fc);
            else                                // Deselect after double-clicking on a model
                eraseSelected( fc);
        }   // end if
        else // Nothing double-clicked
        {
            if ( _exclusive || _selected.size() == _available.size())
                deselectAll();
            else
            {
                for ( FaceControl* f : _available)
                {
                    if ( isSelected(f))
                        continue;
                    insertSelected( fc);
                }   // end foreach
            }   // end else
        }   // end else
    }   // end if
    return swallowed;
}   // end leftDoubleClick


// private
void ModelSelectInteractor::deselectAll()
{
    while ( !_selected.empty())
        eraseSelected( _selected.first());
}   // end deselectAll
