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

#include <ModelSelector.h>
using FaceTools::ModelSelector;
using FaceTools::InteractiveModelViewer;
using FaceTools::FaceControl;


// public
ModelSelector::ModelSelector( InteractiveModelViewer* viewer)
    : _viewer(viewer)
{
    viewer->connectInterface( this);
}   // end ctor


// public
void ModelSelector::add( FaceControl* fcont)
{
    assert( _selected.count(fcont) == 0);
    assert( _available.count(fcont) == 0);
    _available.insert(fcont);
    setSelected( fcont, true);
}   // end add


// public
void ModelSelector::remove( FaceControl* fcont)
{
    assert( _available.count(fcont) > 0);
    setSelected( fcont, false);
    _available.erase(fcont);
}   // end remove


// public
void ModelSelector::setSelected( FaceControl* fcont, bool selected)
{
    _selected.erase(fcont);
    if ( selected)
    {
        assert( _available.count(fcont) > 0);
        _selected.insert(fcont);
    }   // end if
}   // end setSelected


// protected
void ModelSelector::rightButtonDown( const QPoint& p)
{
    FaceControl* fcont = findFromProp( _viewer->getPointedAt());
    if ( fcont && _selected.count( fcont) == 0)
    {
        _selected.insert( fcont);
        emit onSelected( fcont, true);
    }   // end if
}   // end rightButtonDown


// protected
void ModelSelector::leftDoubleClick( const QPoint& p)
{
    FaceControl* fcont = findFromProp( _viewer->getPointedAt());
    if ( fcont)
    {
        if ( _selected.count(fcont) == 0)
        {
            _selected.insert( fcont);
            emit onSelected( fcont, true);
        }   // end if
        else
        {
            _selected.erase( fcont);
            emit onSelected( fcont, false);
        }   // end else
    }   // end if
    else
    {   // Select all toggle!
        const bool addall = _selected.size() < _available.size();
        foreach ( FaceControl* fcont, _available)
        {
            if ( addall && _selected.count(fcont) == 0)
            {
                _selected.insert( fcont);
                emit onSelected( fcont, true);
            }   // end if
            else if ( !addall && _selected.count(fcont) > 0)
            {
                _selected.erase( fcont);
                emit onSelected( fcont, false);
            }   // end else if
        }   // end foreach
    }   // end else
}   // end leftDoubleClick


// private
FaceControl* ModelSelector::findFromProp( const vtkProp* prop) const
{
    FaceControl* sfcont = NULL; // Will be the selected FaceControl.

    if (prop)
    {
        // Search for the model - could hash the props in to make this faster,
        // but since the operation is user driven, and there won't be that many
        // models to search through, this is okay for now.
        foreach ( FaceControl* fcont, _available)
        {
            if ( fcont->belongs(prop))
            {
                sfcont = fcont;
                break;
            }   // end if
        }   // end foreach
    }   // end if

    return sfcont;
}   // end findFromProp

