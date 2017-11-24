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

#include <InteractionInterface.h>
#include <cassert>
using FaceTools::InteractionInterface;

// public
InteractionInterface::InteractionInterface()
    : QObject(), _rbdown(false), _lbdown(false), _mbdown(false), _qinterface(NULL)
{
}   // end ctor


// public
void InteractionInterface::setInterface( FaceTools::ModelViewerQtInterface* qinterface)
{
    if ( _qinterface)
        _qinterface->disconnect(this);
    _qinterface = NULL;

    if ( qinterface)
    {
        connect( qinterface, SIGNAL(onRightButtonDown()), this, SLOT(doOnRightButtonDown()));
        connect( qinterface, SIGNAL(onRightButtonUp()), this, SLOT(doOnRightButtonUp()));
        connect( qinterface, SIGNAL(onMiddleButtonDown()), this, SLOT(doOnMiddleButtonDown()));
        connect( qinterface, SIGNAL(onMiddleButtonUp()), this, SLOT(doOnMiddleButtonUp()));
        connect( qinterface, SIGNAL(onLeftButtonDown()), this, SLOT(doOnLeftButtonDown()));
        connect( qinterface, SIGNAL(onLeftButtonUp()), this, SLOT(doOnLeftButtonUp()));
        connect( qinterface, SIGNAL(onLeftDoubleClick()), this, SLOT(doOnLeftDoubleClick()));
        connect( qinterface, SIGNAL(onMouseMove()), this, SLOT(doOnMouseMove()));
    }   // end if
    _qinterface = qinterface;
}   // end setInterface


// public
const QPoint& InteractionInterface::getMouseCoords() const
{
    assert( _qinterface);
    return _qinterface->getMouseCoords();
}   // end getMouseCoords


// public
bool InteractionInterface::isCameraLocked() const { return _qinterface->isCameraLocked();}
void InteractionInterface::setCameraLocked( bool v) { _qinterface->setCameraLocked(v);}


// private slot
void InteractionInterface::doOnRightButtonDown()
{
    const QPoint& p = _qinterface->getMouseCoords();
    emit mousePressed(p);
    rightButtonDown(p);
    _rbdown = true;
}   // end doOnRightButtonDown


// private slot
void InteractionInterface::doOnRightButtonUp()
{
    const QPoint& p = _qinterface->getMouseCoords();
    rightButtonUp(p);
    _rbdown = false;
}   // end doOnRightButtonUp


// private slot
void InteractionInterface::doOnMiddleButtonDown()
{
    const QPoint& p = _qinterface->getMouseCoords();
    emit mousePressed(p);
    middleButtonDown(p);
    _mbdown = true;
}   // end doOnMiddleButtonDown


// private slot
void InteractionInterface::doOnMiddleButtonUp()
{
    const QPoint& p = _qinterface->getMouseCoords();
    middleButtonUp(p);
    _mbdown = false;
}   // end doOnMiddleButtonUp


// private slot
void InteractionInterface::doOnLeftButtonDown()
{
    const QPoint& p = _qinterface->getMouseCoords();
    emit mousePressed(p);
    leftButtonDown(p);
    _lbdown = true;
}   // end doOnLeftButtonDown


// private slot
void InteractionInterface::doOnLeftButtonUp()
{
    const QPoint& p = _qinterface->getMouseCoords();
    leftButtonUp(p);
    _lbdown = false;
}   // end doOnLeftButtonUp


// private slot
void InteractionInterface::doOnLeftDoubleClick()
{
    const QPoint& p = _qinterface->getMouseCoords();
    leftDoubleClick(p);
}   // end doOnLeftDoubleClick


// private slot
void InteractionInterface::doOnMouseMove()
{
    const QPoint& p = _qinterface->getMouseCoords();
    if ( _lbdown)
        leftDrag(p);
    else if ( _rbdown)
        rightDrag(p);
    else if ( _mbdown)
        middleDrag(p);
    else
        mouseMove(p);
}   // end doOnMouseMove

