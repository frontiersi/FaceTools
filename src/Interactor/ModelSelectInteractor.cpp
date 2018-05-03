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
ModelSelectInteractor::ModelSelectInteractor( ModelViewer* mv, bool ex)
    : ModelViewerInteractor( mv),
     _exclusive(ex), _enabled(true)
{}   // end ctor


// public
void ModelSelectInteractor::enableUserSelect( bool v) { _enabled = v;}
void ModelSelectInteractor::setExclusiveSelect( bool v) { _exclusive = v;}
bool ModelSelectInteractor::isExclusiveSelect() const { return _exclusive;}


// public
void ModelSelectInteractor::add( FaceControl* fcont)
{
    assert( _selected.count(fcont) == 0);
    assert( _available.count(fcont) == 0);
    _available.insert(fcont);
    setSelected( fcont, true);
}   // end add


// public
void ModelSelectInteractor::remove( FaceControl* fcont)
{
    assert( _available.count(fcont) > 0);
    setSelected( fcont, false);
    _available.erase(fcont);
}   // end remove


// public
void ModelSelectInteractor::setSelected( FaceControl* fcont, bool selected)
{
    if ( _exclusive)
        _selected.clear();
    else
        _selected.erase(fcont);

    if ( selected)
    {
        assert( _available.count(fcont) > 0);
        _selected.insert(fcont);
    }   // end if
}   // end setSelected


// public
bool ModelSelectInteractor::isSelected( FaceControl* fc) const
{
    return _selected.count(fc) > 0;
}   // end isSelected


// public
bool ModelSelectInteractor::isAvailable( FaceControl* fc) const
{
    return _available.count(fc) > 0;
}   // end isAvailable


// private
void ModelSelectInteractor::eraseSelected( FaceControl* fcont)
{
    _selected.erase(fcont);
    emit onUserSelected( fcont, false);
}   // end eraseSelected


// private
void ModelSelectInteractor::insertSelected( FaceControl* fcont)
{
    _selected.insert(fcont);
    emit onUserSelected( fcont, true);
}   // end insertSelected


void ModelSelectInteractor::rightButtonDown( const QPoint& p)
{
    if ( !_enabled)
        return;

    FaceControl* fcont = _available.find( viewer()->getPointedAt(p));
    if ( fcont && _selected.count( fcont) == 0)
    {
        if ( _exclusive)    // signal to listeners that currently selected models are being deselected
            deselectAll();
        insertSelected( fcont);
    }   // end if
}   // end rightButtonDown


void ModelSelectInteractor::leftDoubleClick( const QPoint& p)
{
    if ( !_enabled)
        return;

    FaceControl* fcont = _available.find( viewer()->getPointedAt(p));
    if ( fcont)
    {
        if ( _exclusive)
            deselectAll();

        if ( _selected.count(fcont) == 0)   // Select after double-clicking on a model
            insertSelected( fcont);
        else                                // Deselect after double-clicking on a model
            eraseSelected( fcont);
    }   // end if
    else // Nothing double-clicked
    {
        if ( _exclusive || _selected.size() == _available.size())
            deselectAll();
        else
        {
            for ( FaceControl* fcont : _available)
            {
                if ( _selected.count(fcont) > 0)
                    continue;
                insertSelected( fcont);
            }   // end foreach
        }   // end else
    }   // end else
}   // end leftDoubleClick


// private
void ModelSelectInteractor::deselectAll()
{
    while ( !_selected.empty())
        eraseSelected( _selected.first());
}   // end deselectAll
