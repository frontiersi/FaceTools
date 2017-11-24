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

class HandleModifierKeyPress : public QTools::KeyPressHandler
{
public:
    HandleModifierKeyPress( ModelSelector* mselector, int key)
        : _mselector(mselector), _key(key) {}

protected:
    virtual bool handleKeyPress( QKeyEvent* e)
    {
        bool handled = false;
        if ( e->key() == _key)
        {
            handled = true;
            _mselector->setSelecting( e->type() == QEvent::KeyPress);
        }   // end if
        return handled;
    }   // end handleKeyPress

private:
    ModelSelector* _mselector;
    int _key;
};  // end class



// public
ModelSelector::ModelSelector( InteractiveModelViewer* viewer, Qt::Key key)
    : _viewer(viewer), _kph(NULL), _selecting(false), _camLockState(false), _dblclick(false)
{
    viewer->connectInterface( this);
    _kph = new HandleModifierKeyPress( this, key);
    viewer->addKeyPressHandler( _kph);
}   // end ctor


// public
ModelSelector::~ModelSelector()
{
    _viewer->removeKeyPressHandler( _kph);
    delete _kph;
}   // end dtor


// public
void ModelSelector::setSelecting( bool v)
{
    if ( _selecting == v) // Only react to state changes
        return;

    _selecting = v;
    if ( _selecting)
    {
        _selected.clear();
        _camLockState = isCameraLocked();
        setCameraLocked(true);
    }   // end if
    else
    {
        _dblclick = false;
        setCameraLocked( _camLockState);
    }   // end else
}   // end setSelecting


void ModelSelector::leftDoubleClick( const QPoint& p)
{
    _dblclick = true;
    setSelecting(true);
    const vtkProp* prop = _viewer->getPointedAt();
    if ( prop)
        leftButtonDown(p);
    else
        emit onSelectAll();
}   // end leftDoubleClick


void ModelSelector::mouseMove( const QPoint&)
{
    if ( _dblclick)
    {
        const vtkProp* prop = _viewer->getPointedAt();
        if ( prop && _selected.count(prop) == 0)
        {
            _selected.insert(prop);
            onSelected( prop, true);
        }   // end if
    }   // end if
}   // end mouseMove


void ModelSelector::leftButtonDown( const QPoint&)
{
    if ( _selecting)    // Only possible to be selecting with modifier key press
    {
        const vtkProp* prop = _viewer->getPointedAt();
        if ( prop)
        {
            if ( _selected.count(prop) == 0)
            {
                _selected.insert(prop);
                onSelected( prop, true);
            }   // end if
            else
            {
                _selected.erase(prop);
                onSelected( prop, false);
            }   // end else
        }   // end if
    }   // end if
}   // end leftButtonDown


void ModelSelector::leftButtonUp( const QPoint&)
{
    if ( _dblclick)
        setSelecting(false);
}   // end leftButtonUp

