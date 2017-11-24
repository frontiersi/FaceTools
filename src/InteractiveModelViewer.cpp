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

#include <InteractiveModelViewer.h>
#include <ModelViewerVTKInterface.h>
#include <cassert>
using FaceTools::InteractiveModelViewer;
using FaceTools::InteractionInterface;

// private
void InteractiveModelViewer::init()
{
    vtkSmartPointer<FaceTools::ModelViewerVTKInterface> vtkInterface( FaceTools::ModelViewerVTKInterface::New());
    _qviewer->setInteractor(vtkInterface);
    _qinterface = vtkInterface->getQtInterface();
    connect( _qinterface, &ModelViewerQtInterface::requestContextMenu, this, &InteractiveModelViewer::requestContextMenu);
}   // end init

// public
InteractiveModelViewer::InteractiveModelViewer( QTools::VtkActorViewer* qviewer)
    : FaceTools::ModelViewer(qviewer), _qinterface(NULL), _pwidget(NULL)
{
    init();
}   // end ctor


// public
void InteractiveModelViewer::addToLayout( QLayout *layout)
{
    layout->addWidget(_qviewer);
}   // end addToLayout


// public
void InteractiveModelViewer::removeFromLayout( QLayout *layout)
{
    layout->removeWidget(_qviewer);
}   // end removeFromLayout


// public
void InteractiveModelViewer::connectInterface( InteractionInterface* iint) const
{
    assert(iint);
    iint->setInterface(_qinterface);
}   // end connectInterface


// public
void InteractiveModelViewer::disconnectInterface( InteractionInterface* iint) const
{
    assert(iint);
    iint->setInterface(NULL);
}   // end disconnectInterface


// public
bool InteractiveModelViewer::isCameraLocked() const { return _qinterface->isCameraLocked();}
void InteractiveModelViewer::setCameraLocked( bool v) { _qinterface->setCameraLocked(v);}


// public
const vtkProp* InteractiveModelViewer::getPointedAt() const
{
    const QPoint& p = getMouseCoords();
    return FaceTools::ModelViewer::getPointedAt( p);
}   // end getPointedAt


// public
void InteractiveModelViewer::setCursor( QCursor cursor)
{
    FaceTools::ModelViewer::setCursor(cursor);
}   // end setCursor


// public
void InteractiveModelViewer::addKeyPressHandler( QTools::KeyPressHandler* kph)
{
    _qviewer->addKeyPressHandler(kph);  // Pass through to QVTKWidget
}   // end addKeyPressHandler


// public
void InteractiveModelViewer::removeKeyPressHandler( QTools::KeyPressHandler* kph)
{
    _qviewer->removeKeyPressHandler(kph);  // Pass through to QVTKWidget
}   // end removeKeyPressHandler


// public slot
void InteractiveModelViewer::setFullScreen( bool fs)
{
    if ( fs && _pwidget == NULL)    // Go fullscreen
    {
        // Store previous state
        _pwidget = _qviewer->parentWidget();
        _winflags = _qviewer->windowFlags();
        _qviewer->setParent(NULL);
        _qviewer->setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        _qviewer->showMaximized();
    }   // end if
    else if ( !fs && _pwidget != NULL) // Go non-fullscreen only if previously went full screen!
    {
        _qviewer->setParent(_pwidget);
        _pwidget = NULL;
        _qviewer->overrideWindowFlags(_winflags);
        _qviewer->show();
    }   // end else
}   // end setFullScreen


